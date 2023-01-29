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
    unsigned n = strtol(argv[1], NULL, 10);
    for (int i = 0; i <= n; i++) {
        char *p = bn_fib_fast(i);
        char *q = bn_fib_iter(i);
        printf("%d, %s\n", i, p);
    }
    return 0;
}
