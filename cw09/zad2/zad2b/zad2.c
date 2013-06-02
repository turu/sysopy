#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

void * thread_run(void * args) {
	while (1) {
	}

	return NULL;
}

void * thread_run_divbyzero(void * args) {
    int a = 1;
    int b = a / 0;
    printf ("b = %d\n", b);

    return NULL;
}

int main(int argc, char ** argv) {
	pthread_t thread;
	if(pthread_create(&thread, NULL, &thread_run, (void*) NULL) < 0) {
	    printf("Creating toy thread has failed.\n");
	    exit(2);
	}
	printf("Toy thread created.\n");

	pthread_t thread_bug;
	if(pthread_create(&thread_bug, NULL, &thread_run_divbyzero, (void*) NULL) < 0) {
	    printf("Creating bug thread has failed.\n");
	    exit(2);
	}
	printf("Bug thread created.\n");

	sleep(1);

	if(pthread_join(thread, NULL) < 0) {
        printf("Failure joining toy thread.\n");
        exit(6);
    }

    if(pthread_join(thread_bug, NULL) < 0) {
        printf("Failure joining bug thread.\n");
        exit(7);
    }

    printf("Success.\n");

	return 0;
}
