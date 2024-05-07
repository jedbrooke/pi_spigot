#include "Worker.hpp"
#include "modpow.h"

void Worker::SET_PARAMETERS(u_int64_t a, u_int64_t b, int64_t c) {
    Worker_Parameters::a = a;
    Worker_Parameters::b = b;
    Worker_Parameters::c = c;
}

void Worker::SET_N(uint64_t n) {
    Worker_Parameters::n = n;
} 

void Worker::SET_STEP(size_t s) {
        Worker_Parameters::STEP = s;
    }

Worker::Worker(size_t tid) {
    this->tid = tid;
}

fractionalBignum<D> Worker::get_work() {
    return s;
}


Worker::~Worker() {}

Bellard_Worker::Bellard_Worker(size_t tid) : Worker(tid) {};
BBP_Worker::BBP_Worker(size_t tid) : Worker(tid) {};

void BBP_Worker::work() {
    u_int64_t k = this->tid;

        u_int64_t numerator = 0;
        u_int64_t denominator = 0;

        int64_t exponent = 0;

        s.setZero();

        bool keep_looping = true;
        while(keep_looping) {
            denominator = (8 * k) + Worker_Parameters::b;
            exponent = Worker_Parameters::n - k;
            numerator = (exponent > 0) ? modpow16(exponent, denominator) : 1;

            auto p = fb_div<D>(numerator, denominator);
            if (exponent < 0) {
                auto q = fractionalBignum<D>(pow(16, exponent)); 
                keep_looping = not q.isZero();
                p = p * q;
            }

            s += p;

            k += Worker_Parameters::STEP;
        }
}

void Bellard_Worker::work() {
    u_int64_t k = this->tid;

    u_int64_t numerator = 0;
    u_int64_t denominator = 0;

    int64_t exponent = 0;

    s.setZero();

    bool keep_looping = true;
    while(keep_looping) {
        denominator = (Worker_Parameters::a * k) + Worker_Parameters::b;
        exponent = Worker_Parameters::c + Worker_Parameters::n - 1 - (10 * k);
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

        k += Worker_Parameters::STEP;
    }
}