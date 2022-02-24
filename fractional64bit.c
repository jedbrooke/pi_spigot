#include "fractional64bit.h"

#include <math.h>
#include <stdio.h>
#include <memory.h>
#include "utility.h"
#include "digits.h"

int f64b_isnan;
int f64b_isinf;
/* 
    prints a fractional64bit value
*/

#define PREC 20
char* f64btoa(const fractional64bit f) {
    if (f == 0) {
        return ".0";
    }
    if(f64b_isnan) {
        return "nan";
    }
    if(f64b_isinf) {
        return "inf";
    }

    uint8_t digits[64] = {0};
    u_int64_t mask = (1L << 63);
    for (int i = 0; i < 64; i++) {
        if (f & mask) {
            base10add(digits, POWERS_OF_TWO[i]);
        }
        mask >>= 1;
    }
    // find trailing zeros
    int last_nonzero = 0;
    for (int i = 63; i >= 0; i--) {
        if (digits[i] != 0) {
            last_nonzero = (i+1);
            break;
        }
    }
    last_nonzero = last_nonzero < PREC ? last_nonzero : (PREC+1);
    // round last digit
    if(digits[last_nonzero] > 4) {
        uint8_t round[64] = {0};
        round[last_nonzero-1] = 1;
        base10add(digits, round);
    }

    char* str = (char*) malloc(last_nonzero + 2);
    str[0] = '.';
    for (int i = 0; i < last_nonzero; i++) {
        str[i+1] = digits[i] + '0';
    }


    str[last_nonzero + 1] = 0;
    return str;

}

fractional64bit dtof64b(double d) {
    f64b_isnan = 0;
    f64b_isinf = 0;
    if(isinf(d)) {
        fprintf(stderr,"error: received inf\n");
        f64b_isinf = 1;
    }
    if(isnan(d)) {
        fprintf(stderr,"error: received nan\n");
        f64b_isnan = 1;
    }
    if(f64b_isinf || f64b_isnan) {
        return 0;
    }

    d -= floor(d);
    u_int64_t i;
    memcpy(&i, &d, 8);
    
    fractional64bit f;
    int16_t exponent = ((i & 0x7FF0000000000000) >> 52);
    u_int64_t mantissa = (i & 0xFFFFFFFFFFFFF);
    if(exponent == 0 && mantissa == 0) {
        f = 0;
    } else {
        // sign extend the 11 bits to fill a 16 bit signed int
        if(exponent & (1 << 10)) {
            exponent |= 0xF800;
        }
        exponent -= 1023;
        mantissa += 0x10000000000000;
        mantissa <<= (exponent + 12);
        f = mantissa;
    }

    return f;
}

fractional64bit f64bmul(fractional64bit a, fractional64bit b) {
    fractional64bit c = 0;
    u_int64_t mask = (1L << 63);
    while(a) {
        a >>= 1;
        if(b & mask) {
            c += a;
        }
        mask >>= 1;
    }
    return c;
}



fractional64bit f64bmul_int(fractional64bit a, u_int64_t b) {
    fractional64bit c = 0;
    u_int64_t mask = 1;
    while(a) {
        if(b & mask) {
            c += a;
        }
        mask <<= 1;
        a <<= 1;
    }
    return c;
}



fractional64bit f64bdiv(u_int64_t numerator, u_int64_t denominator) {
    f64b_isnan = 0;
    if(denominator == 0) {
        f64b_isnan = 1;
        return 0;
    }
    if(numerator >= denominator) {
        numerator %= denominator;
    }

    fractional64bit q = 0;
    __asm__(
        "movq %1, %%rdx\n"
        "xor %%rax, %%rax\n"
        "movq %2, %%rcx\n"
        "divq %%rcx\n"
        "movq %%rax, %0\n"
        : "=r" (q)
        : "r" (numerator), "r" (denominator)
    );
    return q;
}