#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "bign.h"

#define FIB_DEV "/dev/fibonacci"

int main(int argc, char const *argv[])
{
    for (int k = 0; k <= atoi(argv[1]); k++) {
        bn *bignum = bn_fib_iter(k);
        char *p = bn_tostring(bignum);
        printf("%d, %s\n", k, p);
        free(bignum->nums);
        free(bignum);
    }
    return 0;
}
