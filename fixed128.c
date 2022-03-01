#include "fixed128.h"

#include <string.h>
#include <stdio.h>

ufixed128 uf128add(const ufixed128 a, const ufixed128 b) {
    ufixed128 s;
    int carry = __builtin_add_overflow(a.fraction, b.fraction, &(s.fraction));
    s.integer = a.integer + b.integer + carry;
    return s;
}

ufixed128 uf128sub(const ufixed128 a, const ufixed128 b) {
    ufixed128 s;
    int carry = __builtin_sub_overflow(a.fraction, b.fraction, &(s.fraction));
    s.integer = a.integer - b.integer - carry;
    return s;
}

int uf128gte_int(ufixed128 a, u_int64_t b) {
    if(a.integer == b) {
        return a.fraction > 0;
    }
    return a.integer >= b;
}

// shift left logical (no sign extend)
// positive for left shift
// negative for right shift
ufixed128 uf128shiftlli(const ufixed128 a, int n) {
    ufixed128 r;
    r.integer = a.integer;
    r.fraction = a.fraction;

    if (n > 0) {
        if(n >= 64) {
            r.integer = r.fraction;
            r.fraction = 0L;
            r.fraction <<= (n - 64);
        } else {
            // save n upper bits of fraction
            u_int64_t mask = 0xFFFFFFFFFFFFFFFF >> (64 - n);
            mask <<= (64 - n);
            u_int64_t bits = r.fraction & mask;
            bits >>= (64 - n);
            r.fraction <<= n;
            r.integer <<= n;

            // add the n upper bits from the fraction to the integer
            r.integer += bits;
        }
    }
    if(n < 0) {
        n = abs(n);
        if(n >= 64) {
            r.fraction = r.integer;
            r.integer = 0L;
            r.integer >>= (n - 64);
        } else {
            // save n lower bits of integer
            u_int64_t mask = 0xFFFFFFFFFFFFFFFF << (64 - n);
            mask >>= (64 - n);
            u_int64_t bits = r.integer & mask;
            bits <<= (64 - n);
            r.fraction >>= n;
            r.integer >>= n;

            // add the n lower bits from the integer to the fraction
            r.fraction += bits;
        }
    }

    return r;
}

char* uf128toa(const ufixed128 a) {
    char* str = (char*) malloc(80);
    sprintf(str, "%lu", a.integer);
    int n = strlen(str);
    sprintf(str + n, "%s", f64btoa(a.fraction));
    return str;
}

ufixed128 uf128mul(const ufixed128 a, const ufixed128 b) {
    ufixed128 c = {0,0};

    ufixed128 temp = {0,0};

    // a.int * b.int
    c.integer = a.integer * b.integer;
    
    // a.int * b.frac
    __asm__(
        "mulq %%rbx"
        : "=d" (temp.integer), "=a" (temp.fraction)
        : "a" (a.integer), "b" (b.fraction)
    );

    c = uf128add(c, temp);
    temp.integer = 0;
    temp.fraction = 0;

    // a.frac * b.int
    __asm__(
        "mulq %%rbx"
        : "=d" (temp.integer), "=a" (temp.fraction)
        : "a" (a.fraction), "b" (b.integer)
    );

    c = uf128add(c, temp);
    temp.integer = 0;
    temp.fraction = 0;

    // a.frac * b.frac
    __asm__(
        "mulq %%rbx"
        : "=d" (temp.fraction)
        : "a" (a.fraction), "b" (b.fraction)
    );

    c = uf128add(c, temp);
    
    return c;
}