#include <cstdlib>
#include "fractionalBignum.hpp"
#include "constants.h"

namespace Worker_Parameters {
    static u_int64_t a;
    static u_int64_t b;
    static int64_t c;
    static u_int64_t n;
    static size_t STEP;
}


class Worker
{
protected:
    fractionalBignum<D> s;

public:
    size_t tid;
    Worker(size_t tid);
    fractionalBignum<D> get_work();

    ~Worker();
    
    static void SET_PARAMETERS(u_int64_t a, u_int64_t b, int64_t c);
    static void SET_N(u_int64_t n);
    static void SET_STEP(size_t s);

    virtual void work() = 0;
};

class Bellard_Worker : public Worker {
    public:
    Bellard_Worker(size_t tid); 
    void work();
};

class BBP_Worker : public Worker {
    public:
    BBP_Worker(size_t tid); 
    void work();
};