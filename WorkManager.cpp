#include <thread>
#include "modpow.h"
#include "WorkManager.hpp"


WorkManager::WorkManager(size_t n_threads, METHOD m) {
    this->m = m;
    Worker::SET_STEP(n_threads);
    for (size_t i = 0; i < n_threads; i++) {
        switch (m) {
        case BBP:
            workers.push_back(new BBP_Worker(i));
            break;
        case BELLARD:
            workers.push_back(new Bellard_Worker(i));
            break;
        }
    }
}

void WorkManager::set_n(size_t n) {
    Worker::SET_N(n);
}

fractionalBignum<D> WorkManager::run_component(u_int64_t a, u_int64_t b, int64_t c) {
    Worker::SET_PARAMETERS(a, b, c);
    std::vector<std::thread*> threads;
    for(int i = 0; i < workers.size(); i++) {
        threads.push_back(new std::thread([this, i]() {
            this->workers[i]->work();
        }));
    }
    for(auto &&t : threads) {
        t->join();
    }
    fractionalBignum<D> s;
    for (auto &&w : this->workers) {
        s += w->get_work();
    }
    return s;
}

fractionalBignum<D> WorkManager::run() {
    switch (this->m) {
    case BBP:
        return run_bbp();
        break;
    case BELLARD:
        return run_bellard();
        break;
    }
    fractionalBignum<D> f;
    return f;
}

fractionalBignum<D> WorkManager::run_bbp() {
    auto a = run_component(0, 1, 0) << 2;
    auto b = run_component(0, 4, 0) << 1;
    auto c = run_component(0, 5, 0);
    auto d = run_component(0, 6, 0);
    
    auto res = a - b - c - d;

    return res;
};

fractionalBignum<D> WorkManager::run_bellard() {
    auto a = run_component(10, 1, 2);
    auto b = run_component(10, 3, 0);
    auto c = run_component(4, 1, -1);
    auto d = run_component(10, 5, -4);
    auto e = run_component(10, 7, -4);
    auto f = run_component(10, 9, -6);
    auto g = run_component( 4, 3, -6);

    fractionalBignum<D> res;
    res = a - b - c - d - e + f - g;

    return res;
};

WorkManager::~WorkManager() {}
