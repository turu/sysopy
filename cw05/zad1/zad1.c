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

    printf("pid=%d killed after %ds\n", pid, b/(1<<8));
}

void sighandler(int signum) {
    int tdiff = time(NULL) - stamp;

    printf("pid=%d caught signal %d, total lifetime = %ds\n", getpid(), signum, tdiff);

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
    if(argc < 2 || (forks = atoi(argv[1])) < 1) {
        printf("Program usage: <num of childern>\n");
        return 1;
    }

#ifdef SIGACTION
    handler = malloc(sizeof(struct sigaction));
    parhandler = malloc(sizeof(struct sigaction));
    handler->sa_handler = sighandler;
    parhandler->sa_handler = sighandler_parent;

    sigemptyset(&(handler->sa_mask));
    sigemptyset(&(parhandler->sa_mask));

    sigaction(SIGCHLD, parhandler, NULL);
#else
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
        kill(pids[i], SIGTSTP);
        createChild(i);
    }

    free(pids);
#ifdef SIGACTION
    free(handler);
    free(parhandler);
#endif

    return 0;
}
