#include "utility.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

uint8_t extract_hexit(double a) {
    a -= floor(a);
    a *= 16;
    // right now we are only using the most significant hexit
    // but some of the following are probably accurate too
    // figure out how accurate we can get
    return (uint8_t) a;
}


void print_double_raw(double d) {
    u_int64_t i;
    memcpy(&i, &d, 8);
    char* sign = bintos(i >> 63,1);
    char* exponent = bintos((i & 0x7FF0000000000000) >> 52,11);
    char* mantissa = bintos(i & 0xFFFFFFFFFFFFF,52);
    printf("%s\n",sign);
    printf("%s\n",exponent);
    printf("%s\n",mantissa);

}

char* bintos(u_int64_t i, size_t w) {
    char buf[65];
    buf[0] = 0x00;
    int c = 1;
    while(i) { 
        buf[c] = (i & 1) ? '1' : '0';
        c++;
        i >>= 1;
    }
    for(int j = c; j <= w; j++) {
        buf[j] = '0';
    }

    char* ret = (char*) malloc(w+1);

    for(int j = w; j >= 0; j--) {
        ret[w-j] = buf[j];
    }
    return ret;
}


void base10add(uint8_t accumulator[64], const uint8_t b[64]) {
    int carry = 0;
    for(int i = 63; i >= 0; i--) {
        accumulator[i] += b[i];
        accumulator[i] += carry;
        if (accumulator[i] > 9) {
            accumulator[i] %= 10;
            carry = 1;
        } else {
            carry = 0;
        }
    }
}