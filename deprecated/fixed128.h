#include <stdlib.h>
#include <stdbool.h>


/* 
    profiling has showed that converting between signed and unsigned
    may be a bottleneck, so I will merge the types into one and just have the 
    unsigned versions of the functions ignore the sign, and let the caller
    deal with any consequences (possibly set a over flag etc)
*/


typedef struct ufixed128 {
    u_int64_t integer;
    u_int64_t fraction;
} ufixed128;


typedef struct sfixed128 {
    bool sign;
    u_int64_t integer;
    u_int64_t fraction;
} sfixed128;

#pragma once
#include "fractional64bit.h"


/* Unsigned operations */

ufixed128 uf128add(ufixed128 a, ufixed128 b);

ufixed128 uf128sub(const ufixed128 a, const ufixed128 b);

ufixed128 uf128sub_f64b(const ufixed128 a, const fractional64bit b);

ufixed128 uf128mul(const ufixed128 a, const ufixed128 b);

ufixed128 uf128mul_f64b(const ufixed128 a, const fractional64bit b);

bool uf128gte_int(ufixed128 a, u_int64_t b);

bool uf128gte(ufixed128 a, ufixed128 b);

ufixed128 uf128shiftlli(ufixed128 a, int n);

char* uf128toa(ufixed128 a);

/* signed operations */

sfixed128 f128_make_signed(ufixed128 a, bool sign);

ufixed128 f128_make_unsigned(sfixed128 a);

sfixed128 sf128add(const sfixed128 a, const sfixed128 b);

sfixed128 sf128sub(const sfixed128 a, const sfixed128 b);

sfixed128 sf128mul(const sfixed128 a, const sfixed128 b);



