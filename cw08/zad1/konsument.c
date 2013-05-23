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
#include <math.h>

#include "commons.h"

int sem;
int shm;
void* mem;

void clean(int a) {
    shmdt(mem);
}

double computeDet(int n, double M[MAXMTX][MAXMTX]) {
    int i, j, i_count, j_count, count=0;
	double array[n - 1][n - 1], det=0;

	if (n < 1) {
		puts("Error");
		exit(1);
	}
	if (n == 1) return M[0][0];
	if (n == 2) return (M[0][0] * M[1][1] - M[0][1] * M[1][0]);

	for (count = 0; count < n; count++) {
		i_count=0;
		for (i = 1; i < n; i++) {
			j_count=0;
			for (j = 0; j < n; j++) {
				if(j == count) continue;
				array[i_count][j_count] = M[i][j];
				j_count++;
			}
			i_count++;
		}
		det += pow(-1, count) * M[0][count] * computeDet(n - 1, array);
	}

	return det;
}

void process(command e) {
	double det = computeDet(MAXMTX, e.mat);

	printf("Zadanie otrzymalem, zadanie wykonalem. Wynik to %f.\n", det);

}

void konsumuj(int ur) {
    command e;

	struct sembuf sb;
	sb.sem_flg = 0;

	int* begin_k = mem+sizeof(int);
	command * tab = mem+2*sizeof(int);

	if (ur)
		*begin_k = 0;
	int licznik;

	while (1) {
		sb.sem_num = KONSUMENT;
		sb.sem_op = ZMNIEJSZ;

		semop(sem, &sb, 1);
		sb.sem_num = LICZNIK;
		semop(sem, &sb, 1);

		licznik = *begin_k;
		(*begin_k) = (licznik+1)%MAXTAB;

		e = tab[licznik];
		process(e);
		sb.sem_op = ZWIEKSZ;
		semop(sem, &sb, 1);
		sb.sem_num = PRODUCENT;
		semop(sem, &sb, 1);
	}
}


int main(int argc, char ** argv) {
	key_t klucz;
	int ur;

	signal(SIGINT, clean);

	klucz = ftok(KPATH, KVAL);

	if ((sem = semget(klucz, 3, IPC_CREAT | IPC_EXCL | S_IRWXU)) == -1) {
        ur = 0;
        sem = semget(klucz, 3, 0);
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
	konsumuj(ur);
	clean(0);

	return 0;
}
