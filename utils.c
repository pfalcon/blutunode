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

#include "utils.h"
#include <ctype.h>
#include <stdio.h>

const char *render_enum(int value, const char *names[], int size)
{
    /* Not reenterable! Don't use more than one in one printf */
    static char buf[10];
    if (value < size) {
        return names[value];
    }
    sprintf(buf, "? (%d)", value);
    return buf;
}

static char hexdigit2char(unsigned val)
{
  val += '0';
  if (val > '9') val += 'A' - '0' - 10;
  return (char)val;
}

void hexb2str(uint8 val, char *buf)
{
  *buf++ = hexdigit2char((val >> 4) & 0xf);
  *buf++ = hexdigit2char(val & 0xf);
}

void hexw2str(uint16 val, char *buf)
{
  *buf++ = hexdigit2char(val >> 12);
  *buf++ = hexdigit2char((val >> 8) & 0xf);
  *buf++ = hexdigit2char((val >> 4) & 0xf);
  *buf++ = hexdigit2char(val & 0xf);
}

uint32 get_num_base(const uint8 *s, int len, int base)
{
    uint32 val;
    for (val = 0; len; len--) {
        char d = toupper(*s++) - '0';
        if (d > 9) {
            d -= 'A' - '0' - 10;
        }
        val = val * base + d;
    }
    return val;
}

uint32 get_num(const uint8 *s, int len)
{
    if (*s == '0' && tolower(s[1]) == 'x') {
        return get_num_base(s + 2, len - 2, 16);
    } else {
        return get_num_base(s, len, 10);
    }
}
