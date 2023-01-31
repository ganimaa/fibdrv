#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "bign.h"

#define N 1000
#define SAMPLE 100000


int main(int argc, char const *argv[])
{
    bn *fast = NULL;
    for (int i = 0; i < SAMPLE; i++) {
        fast = bn_fib_fast(N);
    }
    bn_free(fast);
    return 0;
}