#include "modpow.h"

const u_int64_t HIGHEST_1_64 = (u_int64_t) 1 << 63;
const u_int32_t HIGHEST_1_32 = (u_int32_t) 1 << 31;

#ifdef __x86_64__
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
#else

u_int64_t modpow(u_int64_t base, u_int64_t exponent, u_int64_t const mod) {
    u_int64_t result = 1;
    while(exponent > 0) {
        if (exponent & 1) {
            result = ((unsigned __int128) result * base) % (unsigned __int128) mod;

        }
        base = ((unsigned __int128) base * base) % (unsigned __int128) mod;
        exponent >>= 1;
    }


    return result;

}

#endif

u_int64_t modpow16(u_int64_t exponent, u_int64_t const mod) {
    return modpow(16, exponent, mod);
}

u_int64_t modpow2(u_int64_t exponent, u_int64_t const mod) {
    return modpow(2, exponent, mod);
}