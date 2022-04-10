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
u_int64_t modpow(u_int64_t base, u_int64_t exponent, u_int64_t const mod) {
    u_int64_t result = 1;
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



u_int64_t modpow16(u_int64_t exponent, u_int64_t const mod) {
    return modpow(16, exponent, mod);
}

u_int64_t modpow2(u_int64_t exponent, u_int64_t const mod) {
    return modpow(2, exponent, mod);
}

/* precision for fractionalBignum*/
const size_t D = 4;

fractionalBignum<D> component_sum(size_t n, u_int64_t b) {
    fractionalBignum<D> s1;
    for(size_t k = 0; k < n; k++) {
        u_int64_t k8_plus_b = (k << 3L) + b;
        u_int64_t numerator = modpow16(n-k, k8_plus_b);
        auto p = fb_div<D>(numerator, k8_plus_b);
        s1 += p;
    }
    // k==n
    s1 += fb_div<D>(1L, ((n << 3) + b));
    

    // more precision
    fractionalBignum<D> p;
    fractionalBignum<D> s2;
    auto k = 1;
    do {
        auto q = fb_div<D>(1, (8 * (n+k)) + b);
        p = fractionalBignum<D>(pow(2,-4 * k));
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

fractionalBignum<D> component_sum_bellard(size_t n, u_int64_t a, u_int64_t b, int64_t c) {
    fractionalBignum<D> s;
    fractionalBignum<D> prev;
    
    u_int64_t k = 0;

    u_int64_t numerator = 0;
    u_int64_t denominator = 0;

    int64_t exponent = 0;

    bool keep_looping = true;
    while(keep_looping) {
        denominator = (a * k) + b;
        exponent = c + (signed)n - 1 - (10 * k);
        numerator = (exponent > 0) ? modpow2(exponent, denominator) : 1;

        auto p = fb_div<D>(numerator, denominator);
        if (exponent < 0) {
            auto q = fractionalBignum<D>(pow(2, exponent)); 
            keep_looping = not q.isZero();
            p = p * q;
        }

        if (k & 1) {
            s = s - p;
        } else {
            s = s + p;
        }

        k++;
    } 
    return s;
}


fractionalBignum<D> pi_spigot_bellard(size_t n) {
    // std::cout << "finding a" << std::endl;
    auto a = component_sum_bellard(n, 10, 1, 2);
    // std::cout << "finding b" << std::endl;
    auto b = component_sum_bellard(n, 10, 3, 0);
    // std::cout << "finding c" << std::endl;
    auto c = component_sum_bellard(n, 4, 1, -1);
    // std::cout << "finding d" << std::endl;
    auto d = component_sum_bellard(n, 10, 5, -4);
    // std::cout << "finding e" << std::endl;
    auto e = component_sum_bellard(n, 10, 7, -4);
    // std::cout << "finding f" << std::endl;
    auto f = component_sum_bellard(n, 10, 9, -6);
    // std::cout << "finding g" << std::endl;
    auto g = component_sum_bellard(n, 4, 3, -6);

    fractionalBignum<D> res;
    res = a - b - c - d - e + f - g;

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
    options opts = {false, false, 0, 50};

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

    if(opts.progress) {
        std::cerr << "the progress option is not currently implemented" << std::endl;
    }

    return opts;
}

void pi_slice(options opts) {
    const int step_hex = (16 * D) - 4;
    const int step_bin = step_hex * 4;
    int total_steps = 0;
    for(int i = 0; i < opts.range; i+=step_bin) {
        auto d = pi_spigot_bellard(opts.n + i);
        auto str = d.hex_str();
        total_steps += step_bin;
        if(total_steps < opts.range) {
            std::cout << str.substr(0, step_hex);
        } else {
            std::cout << str.substr(0, (opts.range - (total_steps - step_bin)) / 4);
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
        opts.n = (opts.n * 4) + 1;
        opts.range = (opts.range * 4);
        pi_slice(opts);
    }
    return 0;
}


