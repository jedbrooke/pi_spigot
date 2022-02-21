#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

typedef unsigned char uint8_t;

// https://stackoverflow.com/a/8498251
// we can cache the result of these
u_int64_t modpow16(register u_int64_t exponent, register u_int64_t const mod) {
    register u_int64_t base = 16 % mod;
    register u_int64_t result = 1;
    while(exponent > 0) {
        if(exponent & 1) {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exponent >>= 1;
    }
    return result;
}

// u_int64_t modpow16_asm(u_int64_t exponent, u_int64_t const mod) {
//     u_int64_t base = 16 % mod;
//     u_int64_t result = 1;
//     __asm__(
//         "movq $0 %rax"
//         "movq rbx, base"
//         "movq rcx, mod"
//         "begin:"
//         "and rdx, rax, 1"
//         "jz "


//         "next_power:"
//         "mul rbx, rbx"
//         ""
//     );
// }

#ifdef USE_ASM
    #define MODPOW16 modpw16_asm
#else
    #define MODPOW16 modpow16
#endif


#define PRECISION_ITERS 0

double component_sum(size_t n, u_int64_t b) {
    double s1 = 0;
    // grid stride EZ
    for(size_t k = 0; k < n; k++) {
        u_int64_t k8_plus_b = (k << 3) + b;
        u_int64_t numerator = MODPOW16(n-k, k8_plus_b);
        s1 += numerator / (double) k8_plus_b;
        s1 -= floor(s1);
    }
    // k==n
    s1 += 1.0 / ((n << 3) + b);
    

    // more precision
    // I was still getting correct results without this component
    // for a single hexit
    double s2 = 0;
    for(size_t k = 1; k <= PRECISION_ITERS; k++) {
        double numerator = pow((1 << 4), -(signed)k);
        u_int64_t k8_plus_b = ((k+n) << 3) + b;
        s2 += numerator / k8_plus_b;
    }
    // if(b == 1){    
    //     printf("s1:  %0.25f\n",s1);
    //     printf("s2:  %0.25f\n",s2);
    //     printf("sum: %0.25f\n",s1 + s2);
    // }
    s1 += s2;


    return s1;
}



uint8_t extract_hexit(double a) {
    a -= floor(a);
    a *= 16;
    // right now we are only using the most significant hexit
    // but some of the following are probably accurate too
    // figure out how accurate we can get
    return (uint8_t) a;
}

double pi_spigot(size_t n) {
    // if num_threads % 4 = 0
    // one thread per component
    double a = 4 * component_sum(n,1);
    double b = 2 * component_sum(n,4);
    double c = component_sum(n,5);
    double d = component_sum(n,6);
    double res = a - b - c - d;
    // printf("a:  %0.25f\n",a);
    // printf("b:  %0.25f\n",b);
    // printf("c:  %0.25f\n",c);
    // printf("d:  %0.25f\n",d);
    return res;
}



uint8_t pi_spigot_single(size_t n) {
    
    double s = pi_spigot(n);
    return extract_hexit(s);
}

void print_digits(uint8_t* digits, size_t n) {
    for(size_t i = 0; i < n; i++) {
        printf("%d",digits[i]);
    }
}
void print_hexits(uint8_t* hexits, size_t n) {
    for(size_t i = 0; i < n; i++) {
        printf("%0x",hexits[i]);
    }
}


void print_all_hexits(double s) {
    do {
        printf("%0x",extract_hexit(s));
        s *= 16;
        s -= floor(s);

    } while(s > 0);
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


void pi_full(options opts) {
    uint8_t* hexits = (uint8_t*) malloc(opts.n);
    size_t checkpoint = opts.n / 100;
    // find pi
    for(size_t i = 0; i < opts.n; i++) {
        if (i % checkpoint == 0 && opts.progress) {
            fprintf(stderr,"%ld percent done\n", i / checkpoint);
        }
        hexits[i] = pi_spigot_single(i);
        if(recieved_sigint) {
            opts.n = i;
            break;
        }
    }
    printf("3.");
    print_hexits(hexits, opts.n);
    printf("\n");
}


void pi_slice(options opts) {
    for(int i = 0; i < opts.range; i++) {
        uint8_t d = pi_spigot_single(opts.n + i);
        printf("%0x",d);
    }
    printf("\n");
}

int main(int argc, char* const* argv)
{

    options opts = parse_args(argc, argv);


    if (opts.full) {
        signal(SIGINT, handle_sigint);
        printf("3.");
        pi_full(opts);
    } else {
        pi_slice(opts);
    }
    // double s = pi_spigot(opts.n);
    // printf("%0.25f\n",s);
    // print_all_hexits(s);
    // printf("\n");

    return 0;
}


