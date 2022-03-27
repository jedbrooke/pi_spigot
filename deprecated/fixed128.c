#include "fixed128.h"
#include "fractional64bit.h"

#include <string.h>
#include <stdio.h>

ufixed128 uf128add(const ufixed128 a, const ufixed128 b) {
    ufixed128 s;
    int carry = __builtin_add_overflow(a.fraction, b.fraction, &(s.fraction));
    s.integer = a.integer + b.integer + carry;
    return s;
}

ufixed128 uf128add_f64b(const ufixed128 a, const fractional64bit b) {
    ufixed128 s;
    int carry = __builtin_add_overflow(a.fraction, b, &(s.fraction));
    s.integer = a.integer + carry;
    return s;
}

ufixed128 uf128sub(const ufixed128 a, const ufixed128 b) {
    ufixed128 s;
    int carry = __builtin_sub_overflow(a.fraction, b.fraction, &(s.fraction));
    s.integer = a.integer - b.integer - carry;
    return s;
}

ufixed128 uf128sub_f64b(const ufixed128 a, const fractional64bit b) {
    ufixed128 s;
    int carry = __builtin_sub_overflow(a.fraction, b, &(s.fraction));
    s.integer = a.integer - carry;
    return s;
}

bool uf128gte_int(ufixed128 a, u_int64_t b) {
    if(a.integer == b) {
        return a.fraction >= 0;
    }
    return a.integer >= b;
}


// return a >= b
bool uf128gte(ufixed128 a, ufixed128 b) {
    if(a.integer == b.integer) {
        return a.fraction >= b.fraction;
    }
    return a.integer >= b.integer;
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
    // caller has to free this memory
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

ufixed128 uf128mul_f64b(const ufixed128 a, const fractional64bit b) {
    ufixed128 r = {0,0};
    r.fraction = f64bmul(a.fraction, b);
    ufixed128 t;
    __asm__(
        "mulq %%rbx\n"
        : "=d" (t.integer), "=a" (t.fraction)
        : "a" (a.integer), "b" (b) 
    );
    return uf128add(t,r);
}



sfixed128 f128_make_signed(ufixed128 a, bool sign) {
    sfixed128 r = {sign, a.integer, a.fraction};
    return r;
}

ufixed128 f128_make_unsigned(sfixed128 a) {
    ufixed128 r = {a.integer, a.fraction};
    return r;
}

sfixed128 sf128add(const sfixed128 a, const sfixed128 b) {
    sfixed128 r;
    if (a.sign ^ b.sign) {
        if(a.sign == 1) { // a is negative, b is positive
            ufixed128 abs_a = f128_make_unsigned(a);
            ufixed128 abs_b = f128_make_unsigned(b);
            // if a >= b
            // do b - abs(a)
            if(uf128gte(abs_a, abs_b)) {
                r = f128_make_signed(uf128sub(abs_a,abs_b),1);
            } else { 
                // b > a
                // do b - abs(a)
                r = f128_make_signed(uf128sub(abs_b, abs_a),0);
            }
        } else {
            r = sf128add(b, a);
        }
    } else {
        r = f128_make_signed(
            uf128add(
                f128_make_unsigned(a),
                f128_make_unsigned(b)
            ),
            a.sign
        );
    }

    return r;
}

sfixed128 sf128sub(const sfixed128 a, const sfixed128 b) {
    ufixed128 abs_b = f128_make_unsigned(b);
    return sf128add(a, f128_make_signed(abs_b, b.sign ? 0 : 1));
    
}


sfixed128 sf128mul(const sfixed128 a, const sfixed128 b) {
    int sign = a.sign + b.sign;
    ufixed128 abs_a = f128_make_unsigned(a);
    ufixed128 abs_b = f128_make_unsigned(b);
    ufixed128 p = uf128mul(abs_a, abs_b);
    return f128_make_signed(p, sign == 1);
}

