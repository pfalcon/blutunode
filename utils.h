#include <csrtypes.h>

const char *render_enum(int value, const char *names[], int size);
uint32 get_num_base(const uint8 *s, int len, int base);
uint32 get_num(const uint8 *s, int len);
