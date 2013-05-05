#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include "config.h"

int pid;
time_t stamp;

void sighandler_parent(int signum) {
    int b = -1;
    pid_t pid = wait(&b);

#ifdef SIGACTION
    printf("pid=%d received value %d\n", pid, b/(1<<8));
#else
    printf("pid=%d killed after %ds\n", pid, b/(1<<8));
#endif

}

void sighandler(int signum) {
    int tdiff = time(NULL) - stamp;

    printf("pid=%d caught signal %d, total lifetime = %ds\n", getpid(), signum, tdiff);

    _exit(tdiff);
}

void sighandler_queued(int signum, siginfo_t * info, void * context) {
    int tdiff = time(NULL) - stamp;

    printf("pid=%d caught signal %d, total lifetime = %ds\n", getpid(), signum, tdiff);

    if(info->si_code == SI_QUEUE) {
		printf("\tvalue = %d\t send UID = %d\t send PID = %d\n", info->si_value.sival_int, info->si_uid, info->si_pid);

        exit(info->si_value.sival_int);
    }

    _exit(tdiff);
}

int forks;
int * pids;
struct sigaction * handler, * parhandler;

void createChild(int i) {
    int PID = fork();

    if (PID == 0) {
        stamp = time(NULL);
        pid = getpid();
#ifdef SIGACTION
        sigaction(SIGTSTP,handler,NULL);
#else
        signal(SIGTSTP, sighandler);
#endif
        while (1) {
            printf("pid=%d is alive\n", pid);
            sleep(3);
        }
    } else {
        pids[i] = PID;
    }
}

int main(int argc, char** argv) {
#ifdef SIGACTION
    if (argc < 3){
        printf("Program usage: <num of childern> <value>\n");
        return 1;
    }
    forks = atoi(argv[1]);
    int value = atoi(argv[2]);
    if (forks < 1 || value < 0 || value > 127) {
        printf("Program usage: <num of childern> <value>\n");
        return 1;
    }
    handler = malloc(sizeof(struct sigaction));
    parhandler = malloc(sizeof(struct sigaction));
    handler->sa_handler = sighandler;
    parhandler->sa_handler = sighandler_parent;
    handler->sa_sigaction = sighandler_queued;

    sigemptyset(&(handler->sa_mask));
    sigemptyset(&(parhandler->sa_mask));

    sigaction(SIGCHLD, parhandler, NULL);
    handler->sa_flags=SI_QUEUE;
    union sigval sigvalue;
    sigvalue.sival_int=value;
#else
    if(argc < 2 || (forks = atoi(argv[1])) < 1) {
        printf("Program usage: <num of childern>\n");
        return 1;
    }
    signal(SIGCHLD, sighandler_parent);
#endif
    pids = malloc(sizeof(int) * forks);

    int i;
    for(i = 0; i < forks; i++) {
        createChild(i);
    }

    while (1) {
        sleep(1);
        int i = rand() % forks;
#ifdef SIGACTION
        sigqueue(pids[i],SIGTSTP,sigvalue);
        sigvalue.sival_int++;
#else
        kill(pids[i],SIGTSTP);
#endif
        createChild(i);
    }

    free(pids);
#ifdef SIGACTION
    free(handler);
    free(parhandler);
#endif

    return 0;
}
