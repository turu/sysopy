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
FILE* plik;

void clean(int a) {
    shmdt(mem);
	fclose(plik);
}

void suma(command e, int wynik[MAXMTX][MAXMTX]) {
	int i, j;
	for (i = 0; i < MAXMTX; i++) {
		for(j = 0; j < MAXMTX; j++) {
			wynik[i][j] = e.mat1[i][j] + e.mat2[i][j];
		}
	}
}

void roznica(command e, int wynik[MAXMTX][MAXMTX]) {
	int i, j;
	for (i = 0; i < MAXMTX; i++) {
		for (j = 0; j < MAXMTX; j++) {
			wynik[i][j] = e.mat1[i][j] - e.mat2[i][j];
		}
	}
}

void iloczyn(command e, int wynik[MAXMTX][MAXMTX]) {
	int i, j, k, d;
	for (i = 0; i < MAXMTX; i++) {
		for (j = 0; j < MAXMTX; j++) {
			wynik[i][j] = 0;
			for(k = 0; k < MAXMTX; k++) {
				wynik[i][j] += e.mat1[i][k] * e.mat2[k][j];
			}
		}
	}
}

void wypiszmat(int wynik[MAXMTX][MAXMTX]) {
	char buf[(MAXMTX*MAXMTX+MAXMTX+5)*sizeof(int)];
	buf[0] = '\0';

	int i, j;
	for (i = 0; i < MAXMTX; ++i) {
		for (j = 0; j < MAXMTX; ++j) {
			sprintf(buf, "%s %i", buf, wynik[i][j]);
		}
		sprintf(buf, "%s%c", buf, '\n');
	}
	sprintf(buf, "%s%s", buf, "\n\0");
	fwrite(buf, strlen(buf)*sizeof(char), 1, plik);
}

void toWrite(command e, int wynik[MAXMTX][MAXMTX]) {
	char tekst[30] = "Wykonalem ";

	switch(e.oper) {
		case SUMA:
			suma(e, wynik);
			sprintf(tekst, "%s%s", tekst, "sume");
			break;
		case ROZNICA:
			roznica(e, wynik);
			sprintf(tekst, "%s%s", tekst, "roznice");
			break;
		case ILOCZYN:
			iloczyn(e, wynik);
			sprintf(tekst, "%s%s", tekst, "iloczyn");
			break;
	}

	printf("%s\n", tekst);
	sprintf(tekst, "%s%c", tekst, '\n');

	fwrite(tekst, strlen(tekst)*sizeof(char), 1, plik);

	wypiszmat(e.mat1);
	wypiszmat(e.mat2);
	wypiszmat(wynik);
}

void konsumuj(int ur) {
    command e;
	int mat3[MAXMTX][MAXMTX];

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
		toWrite(e, mat3);
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
	char buf[PATH_MAX];
	sprintf(buf, "%s%i", "/tmp/prod", getpid());
	plik = fopen(buf, "w+");
	konsumuj(ur);
	clean(0);

	return 0;
}
