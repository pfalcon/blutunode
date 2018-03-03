/*
 * BluTuNode - Bluetooth sensor/actuator node software
 * Copyright (c) 2018 Jacob Schmidt
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

void command_software_version(Task task);

static BtNodeCommandTask app;

static int input_echo = 1;

void sink_write(Sink sink, const char *buf, int size)
{
    int offset = SinkClaim(sink, size);
    uint8 *dest = SinkMap(sink);
    memcpy(dest + offset, buf, size);
    SinkFlush(sink, size);
}

void sink_write_str(Sink sink, const char *str)
{
    sink_write(sink, str, strlen(str));
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
            *self->buf_ptr++ = c;
            if (c == '\r') {
                break;
            }
        }

        SourceDrop(src, processed_size);

        if (c == '\r') {
            *self->buf_ptr = 0;
            PRINT(("Received: %s\n", self->input_buf));
            process_line(self, StreamSinkFromSource(src), self->input_buf);
            self->buf_ptr = self->input_buf;
        }
    }
}

static void task_handler(Task task, MessageId msg_id, Message msg)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;

#ifdef DEBUG
    dump_message(msg_id, msg);
#endif

    switch (msg_id) {
    case CL_INIT_CFM:
        ConnectionRfcommAllocateChannel(task);
        break;
    case CL_SM_PIN_CODE_IND:
        {
            CAST_TYPED_MSG(CL_SM_PIN_CODE_IND, tmsg);
            ConnectionSmPinCodeResponse(&tmsg->bd_addr, 4, (uint8*)"1234");
        }
        break;


    /* Handle BT2.1 Secure Simple Pairing */
    case CL_SM_REMOTE_IO_CAPABILITY_IND:
        {
            CAST_TYPED_MSG(CL_SM_REMOTE_IO_CAPABILITY_IND, tmsg);
            app.dev_a.lap = tmsg->bd_addr.lap;
            app.dev_a.uap = tmsg->bd_addr.uap;
            app.dev_a.nap = tmsg->bd_addr.nap;
        }
        break;
    case CL_SM_IO_CAPABILITY_REQ_IND:
        {
            ConnectionSmIoCapabilityResponse(&app.dev_a, cl_sm_io_cap_no_input_no_output, FALSE, TRUE, FALSE, 0, 0);
        }
        break;

    case CL_SM_AUTHORISE_IND:
        {
            CAST_TYPED_MSG(CL_SM_AUTHORISE_IND, tmsg);
            ConnectionSmAuthoriseResponse(&tmsg->bd_addr, tmsg->protocol_id, tmsg->channel, tmsg->incoming, TRUE);
        }
        break;


    /* RFCOMM setup */
    case CL_RFCOMM_REGISTER_CFM:
        ConnectionWriteScanEnable(hci_scan_enable_inq_and_page);
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
            command_software_version(task);
        }
        break;


    /* Handle data messages */
    case MESSAGE_MORE_DATA:
        {
            MessageMoreData *tmsg = (MessageMoreData*)msg;
            handle_input_data(self, tmsg->source);
            /*parseSource(tmsg->source, task);*/
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
    case MESSAGE_PIO_CHANGED:
        {
            MessagePioChanged *tmsg = (MessagePioChanged*)msg;
            char buf[20];
            sprintf(buf, "GPIO=%u\r\n", tmsg->state);
            sink_write_str(self->sink, buf);
        }
        break;
    case APP_MESSAGE_POLL:
        {
            command_poll_handle(self);
        }
        break;
    case CL_DM_READ_BT_VERSION_CFM:
        {
            CAST_TYPED_MSG(CL_DM_READ_BT_VERSION_CFM, tmsg);
            command_bt_version_handle(self, tmsg);
        }
        break;
    case CL_DM_LOCAL_VERSION_CFM:
        {
            CAST_TYPED_MSG(CL_DM_LOCAL_VERSION_CFM, tmsg);
            command_local_version_handle(self, tmsg);
        }
        break;
    }
}

int main(void)
{
    app.task.handler = task_handler;
    app.buf_ptr = app.input_buf;
    ConnectionSmDeleteAllAuthDevices(0); /* Our module will not remember pairing */
    MessagePioTask(&app.task);
    ConnectionInit(&app.task);

    MessageLoop();

    return 0;
}
