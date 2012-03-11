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
#include <print.h>
#include <message.h>
#include <connection.h>

static void print_message(MessageId msg_id, Message msg)
{
    switch (msg_id){
    case CL_INIT_CFM:
        PRINT(("CL_INIT_CFM (ConnectLib Initialzied)\n"));
        break;
    }
}


static void task_handler(Task task, MessageId msg_id, Message msg)
{
    print_message(msg_id, msg);
}

int main(void)
{
    static TaskData app;
    app.handler = task_handler;
    ConnectionInit(&app);

    MessageLoop();

    return 0;
}
