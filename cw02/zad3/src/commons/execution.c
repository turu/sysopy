#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/times.h>
#include <time.h>
#include "mymem.h"
#include "execution.h"

#define CLK_TICKS sysconf(_SC_CLK_TCK)

MyStatus * firstStatus = NULL;
MyStatus * prevStatus = NULL;
struct tms * prevTime = 0;
clock_t prevReal = 0;
struct tms firstTime;
clock_t firstReal = 0;

void printMemStatus(MyStatus * status) {
    if (status == NULL) {
        printf("MyMem library has not been initialized yet\n");
        return;
    }

    printf("Number of used descriptors: %d\n", status->usedCount);
    printf("Number of free descriptors: %d\n", status->freeCount);
    printf("The biggest free chunk: %dB\n", (int) status->maxFreeSize);
    printf("The smallest free chunk: %dB\n", (int) status->minFreeSize);
}

void checkpoint() {
    struct tms now;
    times(&now);
    clock_t nowReal = clock();
    MyStatus * nowStatus = getMyStatus();

    if(!prevTime){
        firstReal = nowReal;
        firstTime = now;
    } else {
        printf("Time elapsed from the beginning:\t\tReal %f\tSys %f\tUsr %f\n",
           ((double)(nowReal - firstReal)) / CLOCKS_PER_SEC,
           ((double)(now.tms_stime - firstTime.tms_stime)) / CLK_TICKS,
           ((double)(now.tms_utime - firstTime.tms_utime)) / CLK_TICKS);

        printf("Time elapsed from the previous checkpoint:\tReal %f\tSys %f\tUsr %f\n",
           ((double)(nowReal - prevReal)) / CLOCKS_PER_SEC,
           ((double)(now.tms_stime - prevTime->tms_stime)) / CLK_TICKS,
           ((double)(now.tms_utime - prevTime->tms_utime)) / CLK_TICKS);

    }

    printMemStatus(nowStatus);
    printf("CPU time:\t\t\t\t\tReal %f\tSys %f\tUsr %f\n",
           ((double)nowReal) / CLOCKS_PER_SEC,
           ((double)now.tms_stime) / CLK_TICKS,
           ((double)now.tms_utime) / CLK_TICKS);

    prevTime = &now;
    prevReal = nowReal;

    putchar('\n');
}
