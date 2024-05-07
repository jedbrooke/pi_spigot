#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include "fractionalBignum.hpp"
#include "WorkManager.hpp"

typedef struct options {
    bool full;
    bool progress;
    bool bellard;
    size_t n;
    size_t range;
    int threads;
} options;


options parse_args(int argc, char* const* argv) {
    int option;
    bool error = false;
    options opts = {false, false, false, 0, 50, (int) std::thread::hardware_concurrency()};

    while((option = getopt(argc, argv, "fpbt")) != -1) {
        switch(option) {
            case 'f':
                opts.full = true;
                break;
            case 'p':
                opts.progress = true;
                break;
            case 'b':
                opts.bellard = true;
                break;
            case 't':
                if (argv[optind] != NULL) {
                    if (atoi(argv[optind]) > 0) {
                        opts.threads = atoi(argv[optind]);
                    } else {
                        opts.threads += atoi(argv[optind]);
                        opts.threads = std::max(1, opts.threads);
                    }
                    optind++;
                } else {
                    fprintf(stderr,"error: -t must be used with an argument!\n");
                    error = true;
                }
                break;
            case '?':
                fprintf(stderr,"unkown option!\n");
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
        fprintf(stderr,"unkown extra values!\n");
        error = true;
    }

    if (error) {
        fprintf(stderr,"Usage: ./pi_spigot [-f -p -b] [-t int] n [range (default 50)]\n");
        exit(1);
    }

    if(opts.progress) {
        std::cerr << "the progress option is not currently implemented" << std::endl;
    }

    return opts;
}

void pi_slice(options opts) {
    const int step = (16 * D) - 4;
    int total_steps = 0;
    WorkManager wm(opts.threads, BBP);
    for(int i = 0; i < opts.range; i+=step) {
        wm.set_n(opts.n + i);
        auto d = wm.run();
        auto str = d.hex_str();
        total_steps += step;
        if(total_steps < opts.range) {
            std::cout << str.substr(0, step);
        } else {
            std::cout << str.substr(0, opts.range - (total_steps - step));
        }
    }
    printf("\n");
}

void pi_slice_bellard(options opts) {
    const int step_hex = (16 * D) - 4;
    const int step_bin = step_hex * 4;
    int total_steps = 0;
    WorkManager wm(opts.threads, BELLARD);
    for(int i = 0; i < opts.range; i+=step_bin) {
        wm.set_n(opts.n + i);
        auto d = wm.run();
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
#ifdef __x86_64__
    std::cerr << "running x86" << std::endl;
#else
    std::cerr << "running generic" << std::endl;
#endif


    options opts = parse_args(argc, argv);
    if (opts.threads == 0) {
        opts.threads = std::thread::hardware_concurrency();
    }

    std::cerr << "using " << opts.threads << " threads" << std::endl;

    if (opts.full) {
        printf("3.");
        opts.range = opts.n;
        opts.n = 0;
        pi_slice(opts);
    } else {
        if (opts.bellard) {
            std::cerr << "using bellard's formula" << std::endl;
            opts.n = (opts.n * 4) + 1;
            opts.range = (opts.range * 4);
            pi_slice_bellard(opts);
        } else {
            std::cerr << "using standard BBP formula" << std::endl;
            pi_slice(opts);
        }
    }
    return 0;
}


