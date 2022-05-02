#include "modpow.h"

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