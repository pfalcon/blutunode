/*
 * BtNode - Bluetooth sensor/actuator node software
 * Copyright (c) 2011-2012 Paul Sokolovsky
 *
 * BtNode is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#include "btnode.h"


static int input_echo = 1;

static void sink_write(Sink sink, const char *buf, int size)
{
    int offset = SinkClaim(sink, size);
    uint8 *dest = SinkMap(sink);
    memcpy(dest + offset, buf, size);
    SinkFlush(sink, size);
}

static void sink_write_str(Sink sink, const char *str)
{
    sink_write(sink, str, strlen(str));
}

static void write_int_response(Sink sink, int value)
{
    char buf[20];
    sprintf(buf, "%d\r\n", value);
    sink_write_str(sink, buf);
}

static void write_ok(Sink sink)
{
    sink_write_str(sink, "OK\r\n");
}

static void write_error(Sink sink)
{
    sink_write_str(sink, "ERROR\r\n");
}

static void process_line(BtNodeCommandTask *task, Sink sink, char *line)
{
    sink_write_str(sink, "Received: ");
    sink_write_str(sink, line);
    sink_write_str(sink, "\r\n");
    if (!strcmp(line, "at+temp?")) {
        write_int_response(sink, VmGetTemperature());
    } else if (!strcmp(line, "at+gpio?")) {
        write_int_response(sink, PioGet());
    } else if (!strncmp(line, "at+gpio=", 8)) {
        int bits, mask = atoi(line + 8);
        char *p = strchr(line + 8, ',');
        if (!p) {
            write_error(sink);
            return;
        }
        bits = atoi(p + 1);
        PioSet(mask, bits);
        write_ok(sink);
    } else if (!strcmp(line, "at+gpiodir?")) {
        write_int_response(sink, PioGetDir());
    } else if (!strcmp(line, "at+gpiosbias?")) {
        write_int_response(sink, PioGetStrongBias());
    } else if (!strcmp(line, "at+cts?")) {
        write_int_response(sink, PioGetCts());
    } else if (!strncmp(line, "at+adc", 6)) {
        int channel = line[6] & 0xf;
        if (!AdcRequest((Task)task, channel)) {
            write_error(sink);
        }
    }
}

static void handle_input_data(BtNodeCommandTask *self, Source src)
{
    int size;
    while ((size = SourceSize(src)) > 0) {
        const uint8 *p = SourceMap(src);
        int i, processed_size = 0;
        char c = 0;
        for (i = size; i; i--) {
            c = *p++;
            processed_size++;
            if (c == 0x08 || c == 0x7f) {
                if (self->buf_ptr > self->input_buf) {
                    self->buf_ptr--;
                    /* Need to overwrite with a space for Linux terminal */
                    sink_write(StreamSinkFromSource(src), "\x08 \x08", 3);
                }
                continue;
            }
            if (input_echo) {
                if (c == '\r') {
                    sink_write(StreamSinkFromSource(src), "\r\n", 2);
                } else {
                    sink_write(StreamSinkFromSource(src), (char*)p - 1, 1);
                }
            }
            if (c == '\r') {
                break;
            }
            *self->buf_ptr++ = c;
        }

        SourceDrop(src, processed_size);

        if (c == '\r') {
            *self->buf_ptr = 0;
            PRINT(("Received: %s==\n", self->input_buf));
            process_line(self, StreamSinkFromSource(src), self->input_buf);
            self->buf_ptr = self->input_buf;
        }
    }
}

static void task_handler(Task task, MessageId msg_id, Message msg)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;

    print_message(msg_id, msg);

    switch (msg_id) {
    case CL_INIT_CFM:
        ConnectionRfcommAllocateChannel(task);
        break;
    case CL_RFCOMM_REGISTER_CFM:
        ConnectionWriteScanEnable(hci_scan_enable_inq_and_page);
        break;
    case CL_SM_PIN_CODE_IND:
        {
            CAST_TYPED_MSG(CL_SM_PIN_CODE_IND, tmsg);
            ConnectionSmPinCodeResponse(&tmsg->bd_addr, 4, (uint8*)"1234");
        }
        break;
    case CL_SM_AUTHORISE_IND:
        {
            CAST_TYPED_MSG(CL_SM_AUTHORISE_IND, tmsg);
            ConnectionSmAuthoriseResponse(&tmsg->bd_addr, tmsg->protocol_id, tmsg->channel, tmsg->incoming, TRUE);
        }
        break;
    case CL_RFCOMM_CONNECT_IND:
        {
            CAST_TYPED_MSG(CL_RFCOMM_CONNECT_IND, tmsg);
            ConnectionRfcommConnectResponse(task, TRUE, &tmsg->bd_addr, tmsg->server_channel, NULL);
        }
        break;
    case CL_RFCOMM_CONNECT_CFM:
        {
            CAST_TYPED_MSG(CL_RFCOMM_CONNECT_CFM, tmsg);
            self->sink = tmsg->sink;
        }
        break;
    case MESSAGE_MORE_DATA:
        {
            MessageMoreData *tmsg = (MessageMoreData*)msg;
            handle_input_data(self, tmsg->source);
        }
        break;
    case MESSAGE_ADC_RESULT:
        {
            MessageAdcResult *tmsg = (MessageAdcResult*)msg;
            char buf[20];
            sprintf(buf, "ADC%d=%d (%d)\r\n", tmsg->adc_source, tmsg->reading, tmsg->scaled_reading);
            sink_write_str(self->sink, buf);
        }
        break;
    }
}

int main(void)
{
    static BtNodeCommandTask app;
    app.task.handler = task_handler;
    app.buf_ptr = app.input_buf;
    ConnectionInit(&app.task);

    MessageLoop();

    return 0;
}
