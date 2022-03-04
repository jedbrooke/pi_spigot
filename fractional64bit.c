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

inline fractional64bit f64bmul_uf128(fractional64bit a, ufixed128 b) {
    return f64bmul_int(a, b.integer) + f64bmul(a, b.fraction);
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

const ufixed128 TWO = {2,0};


// based on Goldschmidt division
// https://en.wikipedia.org/wiki/Division_algorithm#Goldschmidt_division
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
    int lzcnt = __builtin_clzl(denominator);
    fractional64bit d_prime = denominator << lzcnt;
    fractional64bit n_prime = numerator << lzcnt;


    ufixed128 f = uf128sub_f64b(TWO, d_prime);
    
    for(int i = 0; i < 6; i++) {
        n_prime = f64bmul_uf128(n_prime, f);
        d_prime = f64bmul_uf128(d_prime, f);
        f = uf128sub_f64b(TWO, d_prime);
    }

    return n_prime;
}

const ufixed128 _48_ON_17 = {2,0xd2d2d2d2d2d2d2d2};
const ufixed128 _32_ON_17 = {1,0xe1e1e1e1e1e1e1e1};
const sfixed128 s_ONE = {0,1,0};
#define ALL_ONES 0xFFFFFFFFFFFFFFFF

// based on Newton–Raphson division
// https://en.wikipedia.org/wiki/Division_algorithm#Newton%E2%80%93Raphson_division
fractional64bit f64bdiv_nr(u_int64_t numerator, u_int64_t denominator) {
    
    
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
    int lzcnt = __builtin_clzl(denominator);
    fractional64bit d_prime_frac = denominator << lzcnt;
    fractional64bit n_prime_frac = numerator << lzcnt;

    ufixed128 d_prime = {0, d_prime_frac};
    ufixed128 n_prime = {0, n_prime_frac};

    sfixed128 x = f128_make_signed(uf128sub(_48_ON_17, uf128mul(_32_ON_17, d_prime)), 0);

    for(int i = 0; i < 4; i++) {
        sfixed128 t = f128_make_signed(uf128mul(d_prime, f128_make_unsigned(x)), 0); 
        sfixed128 u = sf128sub(s_ONE, t);
        sfixed128 v = sf128mul(x, u);
        x = sf128add(x, v);
    }
    ufixed128 q = uf128mul(n_prime, f128_make_unsigned(x));
    return q.fraction;
}


