#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string>

#include "fractionalBignum.hpp"


// https://stackoverflow.com/a/8498251
u_int64_t modpow16(u_int64_t exponent, u_int64_t const mod) {
    u_int64_t result = 1;
    u_int64_t base = 16;
    while (exponent > 0) {
        if (exponent & 1) {
            __asm__(
                "mulq %%rbx\n"
                "divq %%rcx"
                : "=d" (result)
                : "a" (result), "b" (base), "c" (mod)
            );
        }
        __asm__(
            "mulq %%rax\n"
            "divq %%rbx"
            : "=d" (base)
            : "a" (base), "b" (mod) 
        );

        exponent >>= 1;
    }
    return result;
}


/* 
    still experimenting with different division methods
    here are some result for finding hexits 999950 - 1000000
    with f64dbiv:       0m5.512s
    with f64bdiv_gs:    0m5.528s
    with f64bdiv_nr:    0m8.464s
    f64bdiv_nr can probably be optimized more
    the motivation for finding other division schemes is to eventually 
    scale beyond 64 bits. there is no hardware division for such large
    numbers, and we can employ fast multiplication algorithms to accomplish
    division. These methods will scale to higher bits
    currently f64bdiv_nr is more accurate than f64bdiv_gs, but if we run 
    f64bdiv_gs for more iterations it will lose its speed but become as accurate
*/

/* precision for fractionalBignum*/
const size_t D = 16;

fractionalBignum<D> component_sum(size_t n, u_int64_t b) {
    fractionalBignum<D> s1;
    // grid stride EZ
    for(size_t k = 0; k < n; k++) {
        u_int64_t k8_plus_b = (k << 3L) + b;
        u_int64_t numerator = modpow16(n-k, k8_plus_b);
        // std::cout << numerator << " / " << k8_plus_b << std::endl;
        auto p = fb_div<D>(numerator, k8_plus_b);
        // std::cout << p.decimal_str() << std::endl;
        s1 += p;
    }
    // k==n
    s1 += fb_div<D>(1L, ((n << 3) + b));
    

    // more precision
    // I was still getting correct results without this component
    // for 10 hexits
    fractionalBignum<D> p;
    fractionalBignum<D> s2;
    auto k = 1;
    do {
        auto q = fb_div<D>(1, (8 * (n+k)) + b);
        p = fractionalBignum<D>(pow(16,-k));
        s2 += p * q;
        k++;
    } while(not p.isZero());

#ifdef DEBUG
        if(b == 5){    
            std::cout << "s1:\t" << s1 << std::endl;
            std::cout << "s2:\t" << s2 << std::endl;
            std::cout << "sum:\t" << s1 + s2 << std::endl;
        }
#endif
    s1 += s2;


    return s1;
}

fractionalBignum<D> pi_spigot(size_t n) {
    // if num_threads % 4 = 0
    // one thread per component
    auto a = component_sum(n,1) << 2;
    auto b = component_sum(n,4) << 1;
    auto c = component_sum(n,5);
    auto d = component_sum(n,6);
    
    auto res = a - b - c - d;

#ifdef DEBUG
    std::cout << a << std::endl;
    std::cout << b << std::endl;
    std::cout << c << std::endl;
    std::cout << d << std::endl;
#endif


    return res;
}

typedef struct options {
    bool full;
    bool progress;
    size_t n;
    size_t range;
} options;


options parse_args(int argc, char* const* argv) {
    int option;
    bool error = false;
    options opts = {false, false, 50, 50};

    while((option = getopt(argc, argv, "fp")) != -1) {
        switch(option) {
            case 'f':
                opts.full = true;
                break;
            case 'p':
                opts.progress = true;
                break;
            case '?':
                fprintf(stderr,"unkown option!");
                error = true;
                break;
        }
    }

    if (optind < argc) {
        opts.n = atoll(argv[optind]);
        optind++;
    }
    if (optind < argc) {
        opts.range = atoll(argv[optind]);
        optind++;
    }
    if (optind != argc) {
        fprintf(stderr,"unkown extra values!");
        error = true;
    }

    if (error) {
        fprintf(stderr,"Usage: ./pi_spigot [-f -p] n [range (default 50)]");
        exit(1);
    }

    return opts;
}

void pi_slice(options opts) {
    const int step = (D * 16) - 4;
    int total_steps = 0;

    for(int i = 0; i < opts.range; i+=step) {
        auto d = pi_spigot(opts.n + i);
        auto str = d.hex_str();
        total_steps += step;
        if(total_steps < opts.range) {
            std::cout << str.substr(0,step);
        } else {
            std::cout << str.substr(0,opts.range - (total_steps - step));
        }
    }
    printf("\n");
}

int main(int argc, char* const* argv)
{
    options opts = parse_args(argc, argv);

    if (opts.full) {
        printf("3.");
        opts.range = opts.n;
        opts.n = 0;
        pi_slice(opts);
    } else {
        pi_slice(opts);
    }
    return 0;
}


