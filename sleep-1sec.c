#include "main.h"
#include <time.h>   /* nanosleep, clock_nanosleep */
#include <unistd.h> /* sleep, usleep */

int sleep2(int sec) {
    // sleep(sec);
    struct timespec ts = {sec, 0};
#ifdef __linux
    // new glibc uses CLOCK_REALTIME although WSL1 doesn't implement it
    // @see https://qiita.com/mmns/items/eaf42dd3345a2285ff9e
    return clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
#else
    return nanosleep(&ts, NULL);
#endif
}

int main(void) {
    sleep2(1);
    return 0;
}
