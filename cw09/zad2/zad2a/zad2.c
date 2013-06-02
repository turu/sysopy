#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#ifndef SIGNAL
#define SIGNAL SIGUSR1
#endif

void sig_handler(int signo) {
	printf("PID = %d TID = %d\n", getpid(), (int)pthread_self());
}

void * thread_run(void * args) {
	#ifdef V_3
	signal(SIGNAL, sig_handler);
	#elif V_4
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGNAL);
	if(sigprocmask(SIG_SETMASK, &set, NULL) < 0) {
		printf("Masking SIG %d in toy thread failed.\n", SIGNAL);
        exit(1);
    }
    printf("SIG %d masked for the toy thread.\n", SIGNAL);
	#endif

	while (1) {
	}

	return NULL;
}

int main(int argc, char ** argv) {
	#ifdef V_2
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGNAL);
	if(sigprocmask(SIG_SETMASK, &set, NULL) < 0) {
        printf("Masking SIG %d in main thread failed.\n", SIGNAL);
        exit(1);
	}
	printf("SIG %d masked for the main thread.\n", SIGNAL);
	#endif

	pthread_t thread;
	if(pthread_create(&thread, NULL, &thread_run, (void*) NULL) < 0) {
	    printf("Creating toy thread has failed.\n");
	    exit(2);
	}
	printf("Toy thread created.\n");

	sleep(1);

	#if defined V_4 || V_5
	if(pthread_kill(thread, SIGNAL) < 0) {
	    printf("Failure sending signal %d to the toy thread\n", SIGNAL);
	    exit(3);
	}
	printf("Sending signal to the toy thread thread.\n");
	#else
	#ifdef V_3
	signal(SIGNAL, sig_handler);
	#endif
	if(raise(SIGNAL) < 0) {
	    printf("Failure sending signal %d to the main thread\n", SIGNAL);
	    exit(5);
	}
    printf("Sending signal to the main thread.\n");
	#endif

	if(pthread_join(thread, NULL) < 0) {
        printf("Failure joining toy thread.\n");
        exit(6);
    }
    printf("Success.\n");

	return 0;
}
