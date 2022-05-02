#include <vector>
#include "Worker.hpp"
#include "fractionalBignum.hpp"

enum METHOD {
    BBP,
    BELLARD
};

class WorkManager {
private:
    std::vector<Worker*> workers;
    METHOD m;
    
    fractionalBignum<D> run_bbp();
    fractionalBignum<D> run_bellard();
public:
    WorkManager(size_t n_threads, METHOD m);

    void set_n(size_t n);

    fractionalBignum<D> run();

    

    fractionalBignum<D> run_component(u_int64_t a, u_int64_t b, int64_t c);


    ~WorkManager();

};