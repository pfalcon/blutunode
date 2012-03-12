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

#define DEBUG_PRINT_ENABLED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <print.h>
#include <message.h>
#include <connection.h>
#include <source.h>
#include <sink.h>
#include <stream.h>
#include <vm.h>
#include <pio.h>
#include <adc.h>

typedef struct BtNodeCommandTask {
    TaskData task;
    Sink sink;
    char input_buf[80];
    char *buf_ptr;
} BtNodeCommandTask;

#define CAST_TYPED_MSG(msg_id, typed_msg) msg_id##_T *typed_msg = (msg_id##_T*)msg
#define print_status(status) printf("Status: %d\n", status)
#define print_bdaddr(bd_addr) PRINT(("Addr=%x:%x:%lx\n", bd_addr.nap, bd_addr.uap, bd_addr.lap))

void print_message(MessageId msg_id, Message msg);
