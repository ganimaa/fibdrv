#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define FIB_DEV "/dev/fibonacci"

int main()
{
    char write_buf[] = "testing writing";
    int offset = 100; /* TODO: try test something bigger than the limit */

    int fd = open(FIB_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    for (int i = 0; i <= offset; i++) {
        struct timespec t1, t2;
        lseek(fd, i, SEEK_SET);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        long long kt = write(fd, write_buf, 0);
        clock_gettime(CLOCK_MONOTONIC, &t2);
        long long ut =
            (t2.tv_sec * 1e9 + t2.tv_nsec) - (t1.tv_sec * 1e9 + t1.tv_nsec);
        printf("%d %lld %lld %lld\n", i, kt, ut, ut - kt);
    }

    close(fd);
    return 0;
}
