#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "bign.h"

#define FIB_DEV "/dev/fibonacci"


char *bn_fib_iter(unsigned int n)
{
    bn *res = bn_init(1);
    if (n < 2) {
        res->nums[0] = n;
        return bn_tostring(res);
    }

    bn *a, *b;
    a = bn_init(1);
    b = bn_init(1);
    a->nums[0] = 0;
    b->nums[0] = 1;
    res->nums[0] = 1;
    for (int i = 2; i < n; i++) {
        bn_add(a, b, res);
        SWAP(a, res);
        bn_add(a, b, res);
        SWAP(b, a);
    }
    bn_free(a);
    bn_free(b);
    return bn_tostring(res);
}

char *bn_fib_fast(unsigned int n)
{
    bn *fib0 = bn_init(1);
    if (n < 2) {
        fib0->nums[0] = n;
        return bn_tostring(fib0);
    }
    bn *fib1 = bn_init(1);
    fib0->nums[0] = 0;
    fib1->nums[0] = 1;
    bn *k1 = bn_init(1);
    bn *k2 = bn_init(1);
    int k = 32 - __builtin_clz(n);
    for (unsigned int i = 1U << (k - 1); i; i >>= 1) {
        // F(2k) = F(k) * [ 2 * F(k+1) – F(k) ]
        bn_cpy(k1, fib1);
        bn_lshift(k1, 1);
        bn_sub(k1, fib0, k1);
        bn_mul(k1, fib0, k1);
        // F(2k+1) = F(k)^2 + F(k+1)^2
        bn_mul(fib0, fib0, fib0);
        bn_mul(fib1, fib1, fib1);
        bn_add(fib0, fib1, k2);

        bn_cpy(fib0, k1);
        bn_cpy(fib1, k2);
        if (n & i) {
            // bn_cpy(fib0, k2);
            // bn_add(k1, k2, fib1);
            SWAP(fib0, fib1);
            bn_add(fib0, fib1, fib1);
        }  // else {
        //     bn_cpy(fib0, k1);
        //     bn_cpy(fib1, k2);
        // }
    }
    return bn_tostring(fib0);
}

int main(int argc, char const *argv[])
{
    // for (int i = 0; i <= 95; i++) {
    //     char *p = bn_fib_iter(i);
    //     char *q = bn_fib_fast(i);
    //     printf("Fib_iter(%d): %s\n", i, p);
    //     printf("Fib_fast(%d): %s\n", i, q);
    // }
    char *p = bn_fib_fast(94);
    char *q = bn_fib_iter(94);
    printf("%s\n", p);
    printf("%s\n", q);
    return 0;
}
