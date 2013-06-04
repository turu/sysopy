#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#define PHIL_COUNT 5

int philID[PHIL_COUNT];
int currentlyEating = 0;
pthread_mutex_t eatingMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t forks[PHIL_COUNT];
pthread_cond_t canEat = PTHREAD_COND_INITIALIZER;
pthread_t philosopherThreads[PHIL_COUNT];

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

		pthread_mutex_lock(&eatingMutex);
        if(currentlyEating > 3) {
            pthread_cond_wait(&canEat, &eatingMutex);
        }
        currentlyEating++;
		pthread_mutex_unlock(&eatingMutex);

		pickUp(id, leftFork);
		pickUp(id, rightFork);
        isEating[id] = 1;
        usleep(rand() % 100);
        isEating[id] = 0;
		putDown(leftFork);
		putDown(rightFork);

		pthread_mutex_lock(&eatingMutex);
        currentlyEating--;
        if (currentlyEating < 4) {
				pthread_cond_signal(&canEat);
        }
		pthread_mutex_unlock(&eatingMutex);
	}

	return NULL;
}

int main(int argc, char ** argv) {
	srand(time(NULL));

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

	return 0;
}
