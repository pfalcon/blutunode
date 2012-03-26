/*
 * BluTuNode - Bluetooth sensor/actuator node software
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

#include <csrtypes.h>

const char *render_enum(int value, const char *names[], int size);
uint32 get_num_base(const uint8 *s, int len, int base);
uint32 get_num(const uint8 *s, int len);
void hexb2str(uint8 val, char *buf);
void hexw2str(uint16 val, char *buf);
