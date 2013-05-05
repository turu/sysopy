#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <linux/sched.h>
#include<sys/resource.h>

#define STACK_SIZE 1000

int childPID, counter, N, NK;
long childTime, parentTime;

int fn(void * ptr) {
    counter++;
    long t = times(NULL);
    _exit(t- parentTime);
}

void printHelp() {
    printf("Arguments: <number of forks>\n");
}

int main(int argc, char **argv) {
    char * stack = (char*) malloc(STACK_SIZE);
    stack += STACK_SIZE;
    childTime = 0;
    parentTime = 0;
    struct tms tt;
	double clk = (double) sysconf(_SC_CLK_TCK);

    if (argc < 2) {
        printf("y u no give enough arguments?\n");
        printHelp();
        free(stack);
        return 1;
    }

    N = atoi(argv[1]);
    NK = N;

    if (N < 0) {
        printf("y u no give correct number?\n");
        printHelp();
        free(stack);
        return 2;
    }

    long startPoint = times(NULL);

    while (N--) {
        parentTime = times(NULL);
#ifdef FORK
        childPID = fork();
#elif VFORK
        childPID = vfork();
#elif CLONE
        childPID = clone(fn, stack, SIGCHLD, NULL);
#elif VCLONE
        childPID = clone(fn, stack, SIGCHLD | CLONE_VM | CLONE_VFORK, NULL);
#endif
        if (childPID < 0) {
			printf("Blad fork()\n");
			return 3;
		} else if (childPID == 0) {
			fn(NULL);
		} else {
			int status = -1;
			wait(&status);
			childTime += WEXITSTATUS(status);
		}
    }

    long now = times(&tt);
    parentTime = now - startPoint;

    printf("Counter = %d\n\n", counter);

    printf("Parent times:\n");
	printf("real: %lf[s]\n", (double)parentTime / clk);
	printf("user: %lf[s]\n", (double)tt.tms_utime / clk);
	printf("sys:  %lf[s]\n", (double)tt.tms_stime / clk);

	printf("\nChildren times:\n");
	printf("real: %lf[s]\n", (double)childTime / clk);
	printf("user: %lf[s]\n", (double)tt.tms_cutime / clk);
	printf("sys:  %lf[s]\n", (double)tt.tms_cstime / clk);

	double rc = ((double)(childTime))/clk;
    double uc = ((double)(tt.tms_cutime))/clk;
    double sc = ((double)(tt.tms_cstime))/clk;
    double rp = ((double)(parentTime))/clk;
    double up = (double)(tt.tms_utime)/clk;
    double sp = (double)(tt.tms_stime)/clk;
    FILE* fd = fopen("vclone.tmp","a+");
    fprintf(fd,"ChildCount\t%d\tRealTimeSum\t%.2f\tUserTimeSum\t%.2f\tSystemTimeSum\t%.2f\tSys+UsTimeSum\t%.2f\tRealTimeChild\t%.2f\tUserTimeChild\t%.2f\tSystemTimeChild\t%.2f\tSys+UsTimeCh\t%.2f\tRealTimeParent\t%.2f\tUserTimeParent\t%.2f\tSystemTimePar.\t%.2f\tSys+UsTimePar\t%.2f\n",
        NK,rc+rp,uc+up,sc+sp,uc+up+sc+sp,rc,uc,sc,uc+sc,rp,up,sp,up+sp);
    fclose(fd);

    return 0;
}
