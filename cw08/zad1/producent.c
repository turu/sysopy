#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#include "commons.h"

int sem;
int shm;
void* mem;

void clean(int a) {
	shmdt(mem);
	shmctl(shm, IPC_RMID, NULL);
	semctl(sem, IPC_RMID, 0);
}

void produkuj(int ur) {
	srand(time(NULL));

	command e;

	struct sembuf sb;
	sb.sem_flg = 0;

	int* begin_p = mem;
	command * tab = mem+2*sizeof(int);

	if (ur)
		*begin_p = 0;
	int licznik;

	while (1) {
		sb.sem_num = PRODUCENT;
		sb.sem_op = ZMNIEJSZ;

		semop(sem, &sb, 1);

		sb.sem_num = LICZNIK;

		semop(sem, &sb, 1);

		licznik = *begin_p;
		(*begin_p) = (licznik+1)%MAXTAB;

		int i, j;
		for (i = 0; i < MAXMTX; ++i) {
			for (j = 0; j < MAXMTX; ++j) {
				e.mat[i][j] = rand() % 10;
			}
		}

		tab[licznik] = e;

		printf("Zadanie dodane do %i komorki\n", licznik);

		sb.sem_op = ZWIEKSZ;
		semop(sem, &sb, 1);
		sb.sem_num = KONSUMENT;
		semop(sem, &sb, 1);
	}
}


int main(int argc, char* argv[]) {
	int des;
	key_t klucz;
	int ur = 1;

	signal(SIGINT, clean);
    des = open(KPATH, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

	close(des);

	klucz = ftok(KPATH, KVAL);

	if ((sem = semget(klucz, 3, IPC_CREAT | IPC_EXCL | S_IRWXU)) == -1) {
        ur = 0;
        if((sem = semget(klucz, 3, 0)) == -1) {
            printf("%s\n", strerror(errno));
            return -1;
        }
	} else {
		semctl(sem, PRODUCENT, SETVAL, MAXTAB);
		semctl(sem, KONSUMENT, SETVAL, 0);
        semctl(sem, LICZNIK, SETVAL, 1);
	}

	if ((shm = shmget(klucz, MAXTAB*sizeof(command)+2*sizeof(int), IPC_CREAT | IPC_EXCL | S_IRWXU)) == -1) {
        ur = 0;
        if ((shm = shmget(klucz, MAXTAB*sizeof(command)+2*sizeof(int), 0)) == -1) {
            printf("%s\n", strerror(errno));
            return -1;
        }
	}

	mem = shmat(shm, NULL, 0);
	produkuj(ur);
	clean(0);

	return 0;
}
