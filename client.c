#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define FIB_DEV "/dev/fibonacci"
#define SAMPLE 1000


void fib_statistic(int fd, char *buf, char *buf1, int offset)
{
    FILE *f = fopen("fib-fast.log", "w");
    for (int i = offset; i <= offset; i++) {
        lseek(fd, i, SEEK_SET);
        double t1[SAMPLE] = {0}, t2[SAMPLE] = {0};
        double mean1 = 0.0, std1 = 0.0, res1 = 0.0;
        double mean2 = 0.0, std2 = 0.0, res2 = 0.0;

        for (int j = 0; j < SAMPLE; j++) {
            t1[j] = (double) read(fd, buf, 0);
            t2[j] = (double) write(fd, buf1, 0);
            mean1 += t1[j];
            mean2 += t2[j];
        }
        mean1 /= SAMPLE;
        mean2 /= SAMPLE;

        for (int j = 0; j < SAMPLE; j++) {
            std1 += (t1[j] - mean1) * (t1[j] - mean1);
            std2 += (t2[j] - mean2) * (t2[j] - mean2);
        }
        std1 = sqrt(std1 / SAMPLE);
        std2 = sqrt(std2 / SAMPLE);

        int count1 = 0, count2 = 0;
        for (int j = 0; j < SAMPLE; j++) {
            if (t1[j] <= (mean1 + 1.96 * std1) &&
                t1[j] >= (mean1 - 1.96 * std1)) {
                res1 += t1[j];
                count1++;
            }
            if (t2[j] <= (mean2 + 1.96 * std2) &&
                t2[j] >= (mean2 - 1.96 * std2)) {
                res2 += t2[j];
                count2++;
            }
        }
        res1 /= count1;
        res2 /= count2;
        fprintf(f, "%d %.5lf %.5lf samples: %d %d\n", i, res1, res2, count1,
                count2);
    }
    fclose(f);
}

int main()
{
    char buf[1000];
    char write_buf[] = "testing writing";
    int offset = 1000; /* TODO: try test something bigger than the limit */

    int fd = open(FIB_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    // for (int i = 0; i <= offset; i++) {
    //     lseek(fd, i, SEEK_SET);
    //     long long sz = read(fd, buf, 1);
    //     long long uz = write(fd, write_buf, 1);
    //     printf("%d, %lld, %lld\n", i, uz, sz);
    //     // printf("Fib(%3d) = %s execute time: %lld ns\n", i, buf, sz);
    // }

    fib_statistic(fd, buf, write_buf, offset);

    close(fd);
    return 0;
}
