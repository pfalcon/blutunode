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

void command_cts_get(Task task)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;
    write_uint_response(self->sink, PioGetCts());
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

void handleUnrecognised(const uint8 *data, uint16 length, Task task)
{
    BtNodeCommandTask *self = (BtNodeCommandTask*)task;
    write_error(self->sink);           
}
