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

struct InputSource {
    char *name;
    int id;
};

typedef struct BtNodeCommandTask {
    TaskData task;
    Sink sink;
    char input_buf[80];
    char *buf_ptr;
    struct InputSource *poll_source;
    int poll_period;
} BtNodeCommandTask;

/* Periodical poll message */
#define APP_MESSAGE_POLL 10

#define COUNT(arr) (sizeof(arr)/sizeof(*arr))
#define CAST_TYPED_MSG(msg_id, typed_msg) msg_id##_T *typed_msg = (msg_id##_T*)msg
#define print_status(status) printf("Status: %d\n", status)
#define print_bdaddr(bd_addr) PRINT(("Addr=%x:%x:%lx\n", bd_addr.nap, bd_addr.uap, bd_addr.lap))

void print_message(MessageId msg_id, Message msg);

void process_line(BtNodeCommandTask *task, Sink sink, char *line);
void sink_write(Sink sink, const char *buf, int size);
void sink_write_str(Sink sink, const char *str);

/* Handlers for commands with async result */
void command_poll_handle(BtNodeCommandTask *self);
void command_bt_version_handle(BtNodeCommandTask *self, CL_DM_READ_BT_VERSION_CFM_T *tmsg);
void command_local_version_handle(BtNodeCommandTask *self, CL_DM_LOCAL_VERSION_CFM_T *tmsg);
