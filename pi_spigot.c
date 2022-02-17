#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

typedef unsigned char uint8_t;


// https://stackoverflow.com/a/8498251
u_int64_t modpow(u_int64_t base, u_int64_t exponent, u_int64_t mod) {
    base %= mod;
    u_int64_t result = 1;
    while(exponent > 0) {
        if(exponent & 1) {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exponent >>= 1;
    }
    return result;
}


#define PRECISION_ITERS 1

double component_sum(size_t n, u_int64_t b) {

    double s1 = 0;
    for(size_t k = 0; k <= n; k++) {
        u_int64_t k8_plus_b = (k << 3) + b;
        u_int64_t numerator = modpow(16, n-k, k8_plus_b);
        s1 += numerator / (double) k8_plus_b;
    }
    
/* 
    more precision, I was still getting correct results without this component
    double s2 = 0;
    for(size_t k = n+1; k < n + PRECISION_ITERS; k++) {
        u_int64_t numerator = pow(16, n-k);
        u_int64_t k8_plus_b = (k << 3) + b;
        s2 += numerator / (double) k8_plus_b;
    } 
    s1 += s2;
*/


    return s1;

}



uint8_t pi_spigot(size_t n) {
    double a = 4 * component_sum(n,1);
    double b = 2 * component_sum(n,4);
    double c = component_sum(n,5);
    double d = component_sum(n,6);
    double res = a - b - c - d;
    res -= floor(res);
    res *= 16;
    return (uint8_t) res;
}


int main(int argc, char const *argv[])
{
    size_t n = atoi(argv[1]);
    
    // find pi
    for(size_t i = n; i < (n+50); i++) {
        printf("%0x",pi_spigot(i));
    }
    printf("\n");


    return 0;
}


