#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#define SIGNAL SIGUSR1
#define ERROR { int error_code = errno; \
				perror("blad "); \
				exit(error_code); }

void handle(int signo)
{
	printf("PID=%d TID=%d\n", getpid(), (unsigned int)pthread_self());
}

void * fun(void * args)
{
	#ifdef VER_3
	signal(SIGNAL, handle);
	#endif
	#ifdef VER_4
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGNAL);
	if(sigprocmask(SIG_SETMASK, &set, NULL) < 0)
		ERROR;
	#endif
	printf("Thread running\t\t[ OK ]\n");

	while(1)
	{
	}

	printf("Tread exiting . . .\n");

	return NULL;
}

int main(int argc, char * argv[])
{
	#ifdef VER_2
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGNAL);
	if(sigprocmask(SIG_SETMASK, &set, NULL) < 0)
		ERROR;
	#endif

	printf("Creating thread");
	pthread_t thread;
	if(pthread_create(&thread, NULL, &fun, NULL) != 0)
		ERROR;
	printf("\t\t[ OK ]\n");

	//!REPAIR
	sleep(1);

	#ifdef VER_5
	printf("Sending signal to thread");
	if(pthread_kill(thread, SIGNAL) != 0)
		ERROR;
	printf("\t\t[ OK ]\n");
	#elif VER_4
	printf("Sending signal to thread");
	if(pthread_kill(thread, SIGNAL) != 0)
		ERROR;
	printf("\t\t[ OK ]\n");
	#else
	printf("Sending signal to myself");
	if(raise(SIGNAL) < 0)
		ERROR;
	printf("\t\t[ OK ]\n");
	#endif

	printf("Waiting for thread");
	if(pthread_join(thread, NULL) < 0)
		ERROR;
	printf("\t\t[ OK ]\n");

	return EXIT_SUCCESS;
}
