#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string>

#include <immintrin.h>

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

double pi_spigot_avx(size_t n) {
    __m256d s_vec = _mm256_setzero_pd();
    double b[4] = {1,4,5,6};
    double numerators[4];
    double k8_plus_b[4];
    __m256d k8_plus_b_vec;
    __m256d numerators_vec;

    for(size_t k = 0; k < n; k++) {
        for(int i = 0; i < 4; i++) {
            k8_plus_b[i] = (k * 8) + b[i];
            numerators[i] = modpow16(n-k, k8_plus_b[i]);
        }
        k8_plus_b_vec = _mm256_setr_pd(k8_plus_b[0], k8_plus_b[1], k8_plus_b[2], k8_plus_b[3]);
        numerators_vec = _mm256_setr_pd(numerators[0], numerators[1], numerators[2], numerators[3]);
        auto p = _mm256_div_pd(numerators_vec, k8_plus_b_vec);
        s_vec = _mm256_add_pd(p, s_vec);
        s_vec = _mm256_sub_pd(s_vec, _mm256_floor_pd(s_vec));
    }

    double s[4];
    memcpy(s, &s_vec, 4 * sizeof(double));
    s[0] *= 4;
    s[1] *= 2;
    double res = s[0] - s[1] - s[2] - s[3];
    return res;
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

uint8_t extract_hexit(double a) {
    a -= floor(a);
    a *= 16;
    // right now we are only using the most significant hexit
    // but some of the following are probably accurate too
    // figure out how accurate we can get
    return (uint8_t) a;
}

void print_all_hexits(double s) {
    do {
        printf("%0x",extract_hexit(s));
        s *= 16;
        s -= floor(s);

    } while(s > 0);
}

std::string extract_n_hexits(double s, size_t n) {
    std::stringstream ss;
    char* hex = "0123456789abcdef";
    for(size_t i = 0; i < n; i++) {
        ss <<  hex[extract_hexit(s)];
        s *= 16;
        s -= floor(s);
    }
    return ss.str();
}

void pi_slice_avx(options opts) {
    int step = (opts.n > 1E5) ? 10 : 2;
    for(int i = 0; i < opts.range; i+= step) {
        std::cout << extract_n_hexits(pi_spigot_avx(opts.n + i), step);
    }
    std::cout << std::endl;
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
        pi_slice_avx(opts);
    }
    return 0;
}


