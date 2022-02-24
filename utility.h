#include <stdlib.h>

typedef unsigned char uint8_t;


uint8_t extract_hexit(double a);

void print_double_raw(double d);

char* bintos(u_int64_t i, size_t w);

void base10add(uint8_t accumulator[64], const uint8_t b[64]);