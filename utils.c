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
