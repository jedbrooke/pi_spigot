#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <sstream>
#include <string>

#include "utility.h"
#include "fractionalBignum.hpp"


// https://stackoverflow.com/a/8498251
// we can cache the result of these
u_int64_t modpow16(register u_int64_t exponent, register u_int64_t const mod) {
    register u_int64_t base = 16 % mod;
    register u_int64_t result = 1;
    while(exponent > 0) {
        if(exponent & 1L) {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exponent >>= 1L;
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
const size_t K = 4;

fractionalBignum<K> component_sum(size_t n, u_int64_t b) {
    fractionalBignum<K> s1;
    // grid stride EZ
    for(size_t k = 0; k < n; k++) {
        u_int64_t k8_plus_b = (k << 3L) + b;
        u_int64_t numerator = modpow16(n-k, k8_plus_b);
        s1 += div_gs<K>(numerator, k8_plus_b);
    }
    // k==n
    s1 += div_gs<K>(1L, ((n << 3) + b));
    

    // more precision
    // I was still getting correct results without this component
    // for 10 hexits
    // fractional64bit s2 = 0;

    // fractional64bit p = (1L << 63); // 1/2
    // fractional64bit q = 1L;
    // u_int64_t f = 16;
    // for(size_t k = 0; k < 16; k++) {
    //     u_int64_t k8_plus_b = ((k+n) << 3) + b;
    //     q = f64bdiv(1, k8_plus_b * f);
    //     s2 += q;
    //     f *= f;
    // }

#ifdef DEBUG
        if(b == 1){    
            printf("s1:  %#018lx\n",s1);
            printf("s2:  %#018lx\n",s2);
            printf("sum: %#018lx\n",s1 + s2);
        }
#endif
    // s1 += s2;


    return s1;
}

fractionalBignum<K> pi_spigot(size_t n) {
    // if num_threads % 4 = 0
    // one thread per component
    auto a = component_sum(n,1) << 2;
    auto b = component_sum(n,4) << 1;
    auto c = component_sum(n,5);
    auto d = component_sum(n,6);
    
    auto res = a - b - c - d;

#ifdef DEBUG
    std::cout << a << std:endl;
    std::cout << b << std:endl;
    std::cout << c << std:endl;
    std::cout << d << std:endl;
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

sig_atomic_t recieved_sigint = false;
void handle_sigint(int p){
    recieved_sigint = true;
}


// void pi_full(options opts) {
//     uint8_t* hexits = (uint8_t*) malloc(opts.n);
//     size_t checkpoint = opts.n / 100;
//     // find pi
//     for(size_t i = 0; i < opts.n; i++) {
//         if (i % checkpoint == 0 && opts.progress) {
//             fprintf(stderr,"%ld percent done\n", i / checkpoint);
//         }
//         hexits[i] = pi_spigot_single(i);
//         if(recieved_sigint) {
//             opts.n = i;
//             break;
//         }
//     }
//     printf("3.");
//     print_hexits(hexits, opts.n);
//     printf("\n");
// }


void pi_slice(options opts) {

    // const int step = opts.n > 1E5 ? ((64 * K) - 6) : 2;
    // // std::cout << step << std::endl;
    // for(int i = 0; i < opts.range; i+=step) {
    //     auto d = pi_spigot(opts.n + i);
    //     auto str = d.hex_str();
    //     std::cout << str.substr(0,step);
    // }
    auto d = pi_spigot(opts.n);
    std::cout << d.hex_str();
    printf("\n");
}

int main(int argc, char* const* argv)
{

    options opts = parse_args(argc, argv);

#ifdef DEBUG
        printf("n = %ld\n",opts.n);
        // int step = 10;
        // for(size_t i = opts.n; i < opts.n + opts.range; i+=step) {
        //     fractional64bit s = pi_spigot(i);
        //     char str[17];
        //     sprintf(str,"%0lx",s);
        //     str[step] = 0;
        //     printf("%s",str);
        // }
        fractional64bit s = pi_spigot(opts.n);
        printf("%0lx\n",s);
        printf("\n");
#else
    if (opts.full) {
        signal(SIGINT, handle_sigint);
        printf("3.");
        // pi_full(opts);
    } else {
        pi_slice(opts);
    }
#endif
    return 0;
}


