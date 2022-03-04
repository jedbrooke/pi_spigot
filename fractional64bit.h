#include <stdlib.h>


typedef u_int64_t fractional64bit;

#pragma once
#include "fixed128.h"

fractional64bit dtof64b(double d);

fractional64bit f64bmul(fractional64bit a, fractional64bit b);

fractional64bit f64bmul_int(fractional64bit a, u_int64_t b);

fractional64bit f64bmul_uf128(fractional64bit a, ufixed128 b);

fractional64bit f64bdiv(u_int64_t numerator, u_int64_t denominator);

fractional64bit f64bdiv_gs(u_int64_t numerator, u_int64_t denominator);

fractional64bit f64bdiv_nr(u_int64_t numerator, u_int64_t denominator);

char* f64btoa(const fractional64bit f);