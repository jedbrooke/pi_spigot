#include "fractional64bit.h"
#include "fixed128.h"

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
    
    fractional64bit p;
    __asm__(
        "mulq %%rbx\n"
        : "=d" (p)
        : "a" (a), "b"(b)
    );
    return p;
}



fractional64bit f64bmul_int(fractional64bit a, u_int64_t b) {
    return a * b;
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
        "xor %%rax, %%rax\n"
        "divq %%rcx\n"
        : "=a" (q)
        : "d" (numerator), "c" (denominator)
    );
    return q;
}

// based on Goldschmidt division
// https://en.wikipedia.org/wiki/Division_algorithm#Goldschmidt_division
const ufixed128 TWO = {2,0};


fractional64bit f64bdiv_gs(u_int64_t numerator, u_int64_t denominator) {
    if(denominator == 0) {
        f64b_isnan = 1;
        return 0;
    }

    if(numerator > denominator) {
        numerator %= denominator;
    }
    if(numerator == 0) {
        return 0;
    }
    if (denominator == 1) {
        return 0;
    }

    // scale n and d to be [0.5,1]
    // fractional64bit n_prime = numerator << __builtin_clzl(numerator); 
    int lzcnt = __builtin_clzl(denominator);
    fractional64bit d_prime = denominator << lzcnt;
    
    ufixed128 d = {0,d_prime};
    ufixed128 n = {numerator, 0};
    n = uf128shiftlli(n, -(64 - lzcnt));


    ufixed128 f = uf128sub(TWO, d);
    
    for(int i = 0; i < 6; i++) {
        n = uf128mul(n, f);
        d = uf128mul(d, f);
        f = uf128sub(TWO, d);
    }

    return n.fraction;
}
