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

#include <stdio.h>
#include <message.h>
#include <connection.h>

static void task_handler(Task task, MessageId msg_id, Message msg)
{
    printf("Msg: %x\n", msg_id);
}

int main(void)
{
    static TaskData app;
    app.handler = task_handler;
    ConnectionInit(&app);

    MessageLoop();

    return 0;
}
