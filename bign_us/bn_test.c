#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "bign.h"

#define SAMPLE 1000



int main(int argc, char const *argv[])
{
    unsigned int n = strtol(argv[1], NULL, 10);
    for (int i = 0; i <= n; i++) {
        struct timespec t1, t2, t3;
        double time1[SAMPLE] = {0}, time2[SAMPLE] = {0};
        double mean1 = 0.0, std1 = 0.0, res1 = 0.0;
        double mean2 = 0.0, std2 = 0.0, res2 = 0.0;
        bn *iter = NULL, *fast = NULL;
        for (int k = 0; k < SAMPLE; k++) {
            clock_gettime(CLOCK_MONOTONIC, &t1);
            iter = bn_fib_iter(i);
            clock_gettime(CLOCK_MONOTONIC, &t2);
            fast = bn_fib_fast(i);
            clock_gettime(CLOCK_MONOTONIC, &t3);
            double it = (double) (t2.tv_sec * 1e9 + t2.tv_nsec) -
                        (t1.tv_sec * 1e9 + t1.tv_nsec);
            double fd = (double) (t3.tv_sec * 1e9 + t3.tv_nsec) -
                        (t2.tv_sec * 1e9 + t2.tv_nsec);
            time1[k] = it;
            time2[k] = fd;
            mean1 += time1[k];
            mean2 += time2[k];
        }
        mean1 /= SAMPLE;
        mean2 /= SAMPLE;

        for (int k = 0; k < SAMPLE; k++) {
            std1 += (time1[k] - mean1) * (time1[k] - mean1);
            std2 += (time2[k] - mean2) * (time2[k] - mean2);
        }
        std1 = sqrt(std1 / SAMPLE);
        std2 = sqrt(std2 / SAMPLE);

        int count1 = 0, count2 = 0;
        for (int k = 0; k < SAMPLE; k++) {
            if (time1[k] <= (mean1 + 1.96 * std1) &&
                time1[k] >= (mean1 - 1.96 * std1)) {
                res1 += time1[k];
                count1++;
            }
            if (time2[k] <= (mean2 + 1.96 * std2) &&
                time2[k] >= (mean2 - 1.96 * std2)) {
                res2 += time2[k];
                count2++;
            }
        }
        res1 /= count1;
        res2 /= count2;
        printf("%d, %.5lf, %.5lf samples: %d, %d\n", i, res1, res2, count1,
               count2);
        // printf("Fib_iter(%d) = %s\n", i, bn_tostring(iter));
        // printf("Fib_fast(%d) = %s\n", i, bn_tostring(fast));
    }
    return 0;
}
