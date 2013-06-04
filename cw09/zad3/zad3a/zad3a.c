#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#define PHIL_COUNT 5

int philID[PHIL_COUNT];
pthread_mutex_t forks[PHIL_COUNT];
pthread_t philosopherThreads[PHIL_COUNT];
sem_t freeForksSemaphore;

int isEating[PHIL_COUNT];
int forkOwner[PHIL_COUNT];

void printTable(){
	printf("Eating philosophers' ids: ");
	int i;
	for (i = 0; i < PHIL_COUNT; i++) {
		if (isEating[i]) {
			printf("%d, ", i);
		}
	}
	printf("\nFork owners: ");
	for (i = 0; i < PHIL_COUNT; i++) {
		if (forkOwner[i] != -1) {
			printf("\n\t<<Fork %d owned by %d>>", i, forkOwner[i]);
		} else {
		    printf("\n\t<<Fork %d is free>>", i);
		}
    }
	printf("\n\n");
}

void pickUp(int id, int forkID) {
	pthread_mutex_lock(&forks[forkID]);
	forkOwner[forkID] = id;
}

void putDown(int forkID) {
	forkOwner[forkID] = -1;
    pthread_mutex_unlock(&forks[forkID]);
}

void * philosopherRun(void * arg){
	int id = *((int*) arg);

	//every philosopher has his forks
	int leftFork = id;
	int rightFork = (id + 1) % PHIL_COUNT;

	while (1) {
		usleep(rand() % 1000);

		sem_wait(&freeForksSemaphore);
		pickUp(id, leftFork);
		pickUp(id, rightFork);

        isEating[id] = 1;
        usleep(rand() % 100);
        isEating[id] = 0;

		putDown(leftFork);
		putDown(rightFork);
		sem_post(&freeForksSemaphore);
	}

	return NULL;
}

int main(int argc, char ** argv) {
	srand(time(NULL));

	sem_init(&freeForksSemaphore, 0, PHIL_COUNT / 2); //waiter

	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);

	int i;
	for (i = 0; i < PHIL_COUNT; i++) {
		philID[i] = i; //setting hierarchy of resources
		isEating[i] = 0;
		forkOwner[i] = -1;
        pthread_mutex_init(&forks[i], &attr);
	}

	for (i = 0; i < PHIL_COUNT; i++) {
		pthread_create(&philosopherThreads[i], NULL, philosopherRun, &philID[i]);
	}

	while (1) {
		printTable();
		usleep(100000);
	}

	for (i = 0; i < PHIL_COUNT; i++) {
		pthread_join(philosopherThreads[i], NULL);
	}

	sem_destroy(&freeForksSemaphore);

	return 0;
}
