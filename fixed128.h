#include <stdlib.h>
#include "fractional64bit.h"

typedef struct ufixed128 {
    u_int64_t integer;
    u_int64_t fraction;
} ufixed128;

ufixed128 uf128add(ufixed128 a, ufixed128 b);

ufixed128 uf128sub(const ufixed128 a, const ufixed128 b);

ufixed128 uf128mul(const ufixed128 a, const ufixed128 b);

int uf128gte_int(ufixed128 a, u_int64_t b);

ufixed128 uf128shiftlli(ufixed128 a, int n);

char* uf128toa(ufixed128 a);
