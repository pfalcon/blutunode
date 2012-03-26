#include <sink.h>

void write_uint_response(Sink sink, uint16 value);
void write_uint32_response(Sink sink, uint32 value);
void write_response(Sink sink, const char *format, ...);
void write_ok(Sink sink);
void write_ok_uint(Sink sink, uint32 value);
void write_error(Sink sink);
