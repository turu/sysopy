#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/times.h>
#include <time.h>
#include "mymem.h"
#include "execution.h"


MyStatus * firstStatus = NULL;
MyStatus * prevStatus = NULL;

char initialized = 0;
long startTime = 0;
float realStart, sysStart, usrStart, realPrev, sysPrev, usrPrev;
long clk = 0;

void printMemStatus(MyStatus * status) {
    if (status == NULL) {
        printf("MyMem library has not been initialized yet\n");
        return;
    }

    printf("Number of used descriptors: %d\n", status->usedCount);
    printf("Number of free descriptors: %d\n", status->freeCount);
    printf("The biggest free chunk:  %dB\n", (int) status->maxFreeSize);
    printf("The smallest free chunk: %dB\n", (int) status->minFreeSize);
}

void checkpoint() {
    long t;
    struct tms tt;
    float real, usr, sys;
    if (!initialized) {
        startTime = times(NULL);
        clk = sysconf(_SC_CLK_TCK);
    }
    t = times(&tt);
    real = (float)((t - startTime) / (float)(clk));
    sys  = (float)((tt.tms_stime) / (float)(clk));
    usr  = (float)((tt.tms_utime) / (float)(clk));

    if(!initialized){
        realStart = real;
        sysStart = sys;
        usrStart = usr;
        initialized = 1;
    }

    #ifndef DLL
    MyStatus * nowStatus = getMyStatus();
    #endif

    if (initialized) {
        printf("Time elapsed from the beginning:\t\tReal %f\tSys %f\tUsr %f\n",
               real - realStart,
               sys - sysStart,
               usr - usrStart);

        printf("Time elapsed from the previous checkpoint:\tReal %f\tSys %f\tUsr %f\n",
               real - realPrev,
               sys - sysPrev,
               usr - usrPrev);
    }

    #ifndef DLL
    printMemStatus(nowStatus);
    #endif
    printf("CPU time:\t\t\t\t\tReal %f\tSys %f\tUsr %f\n",
           real,
           sys,
           usr);

    realPrev = real;
    sysPrev = sys;
    usrPrev = usr;

    putchar('\n');
}
