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

#include <ctype.h>
#include "btnode.h"
#include "command_parse.h"

enum { 
    ADC0 = VM_ADC_SRC_AIO0,
    ADC1 = VM_ADC_SRC_AIO1,
    ADC2 = VM_ADC_SRC_VREF,
    ADC3 = VM_ADC_SRC_AIO2,
    ADC4 = VM_ADC_SRC_AIO3,
    ADC5 = VM_ADC_SRC_VDD_BAT,

    GPIO = 10,
    TEMP = 20
};

static struct InputSource INPUT_NAMES[] = {
    {"ADC0", ADC0},
    {"ADC1", ADC1},
    {"ADC2", ADC2},
    {"ADC3", ADC3},
    {"ADC4", ADC4},
    {"ADC5", ADC5},
    {"GPIO", GPIO},
    {"TEMP", TEMP},
    {0}
};

static void write_uint_response(Sink sink, uint16 value)
{
    char buf[20];
    sprintf(buf, "%u\r\n", value);
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

/* 1 if result immediately available,
   0 if delayed and handled asynchronously,
  -1 if error */
static int get_input_reading(Task task, int id, uint16 *value)
{
    if (id <= ADC5) {
        if (!AdcRequest(task, id)) {
            return -1;
        }
        return 0;
    }

    switch (id) {
    case GPIO:
        *value = PioGet();
        return 1;
    case TEMP:
        *value = VmGetTemperature();
        return 1;
    }

    return -1;
}

void command_gpio_get(Task task)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;
    write_uint_response(self->sink, PioGet());
}

void command_gpio_set(Task task, const struct command_gpio_set *args)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;
    PioSet(args->mask, args->bits);
    write_ok(self->sink);
}

void command_gpio_pin_get(Task task, const struct command_gpio_pin_get *args)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;
    write_uint_response(self->sink, !!(PioGet() & (1 << args->pin)));
}

void command_gpio_pin_set(Task task, const struct command_gpio_pin_set *args)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;
    uint16 mask = 1 << args->pin;
    char value = *args->value.data;
    if (tolower(value) == 't') {
        PioSet(mask, PioGet() ^ mask);
    } else {
        PioSet(mask, value == '0' ? 0 : mask);
    }
    write_ok(self->sink);
}

void command_gpiodir_get(Task task)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;
    write_uint_response(self->sink, PioGetDir());
}

void command_gpiodir_set(Task task, const struct command_gpiodir_set *args)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;
    PioSetDir(args->mask, args->bits);
    write_ok(self->sink);
}

void command_gpiodir_pin_get(Task task, const struct command_gpiodir_pin_get *args)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;
    write_uint_response(self->sink, !!(PioGetDir() & (1 << args->pin)));
}

void command_gpiodir_pin_set(Task task, const struct command_gpiodir_pin_set *args)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;
    uint16 mask = 1 << args->pin;
    PioSetDir(mask, args->value ? mask : 0);
    write_ok(self->sink);
}

void command_gpiosbias_get(Task task)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;
    write_uint_response(self->sink, PioGetStrongBias());
}

void command_gpiosbias_set(Task task, const struct command_gpiosbias_set *args)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;
    PioSetStrongBias(args->mask, args->bits);
    write_ok(self->sink);
}

void command_gpiosbias_pin_get(Task task, const struct command_gpiosbias_pin_get *args)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;
    write_uint_response(self->sink, !!(PioGetStrongBias() & (1 << args->pin)));
}

void command_gpiosbias_pin_set(Task task, const struct command_gpiosbias_pin_set *args)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;
    uint16 mask = 1 << args->pin;
    PioSetStrongBias(mask, args->value ? mask : 0);
    write_ok(self->sink);
}

void command_gpio_watch_set(Task task, const struct command_gpio_watch_set *args)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;
    PioDebounce(args->mask, args->count, args->period);
    write_ok(self->sink);
}

void command_cts_get(Task task)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;
    write_uint_response(self->sink, PioGetCts());
}

void command_rts_set(Task task, const struct command_rts_set *args)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;
    if (PioSetRts(!!args->value)) {
        write_ok(self->sink);
    } else {
        write_error(self->sink);
    }
}

void command_adc_get(Task task, const struct command_adc_get *args)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;
    if (!AdcRequest(task, args->channel)) {
        write_error(self->sink);
    }
}

void command_temp_get(Task task)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;
    write_uint_response(self->sink, VmGetTemperature());
}

void command_poll_set(Task task, const struct command_poll_set *args)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;
    struct InputSource *p;
    PRINT(("in poll: %s=\n", args->input.data));
    for (p = INPUT_NAMES; p->name; p++) {
        if (!strncmp((char*)args->input.data, p->name, args->input.length)) {
            self->poll_source = p;
            self->poll_period = args->period;
            MessageSend(task, APP_MESSAGE_POLL, NULL);
            write_ok(self->sink);
            return;
        }
    }
    write_error(self->sink);           
}

void command_poll_reset(Task task)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;
    self->poll_source = NULL;
    self->poll_period = 0;
    write_ok(self->sink);
}

void command_poll_handle(BtNodeCommandTask *self)
{
    uint16 value = 0;
    int status;
    char buf[20];

    if (!self->poll_source) {
        return;
    }
    status = get_input_reading((Task)self, self->poll_source->id, &value);
    if (status > 0) {
        sprintf(buf, "%s=%u\r\n", self->poll_source->name, value);
    } else if (status < 0) {
        sprintf(buf, "%s=ERROR\r\n", self->poll_source->name);
    }
    if (status != 0) {
        sink_write_str(self->sink, buf);
    }
    MessageSendLater((Task)self, APP_MESSAGE_POLL, NULL, self->poll_period);
}

void handleUnrecognised(const uint8 *data, uint16 length, Task task)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;
    write_error(self->sink);           
}
