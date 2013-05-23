#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#include "types.h"

int sem;
int shm;
void* mem;
FILE* plik;

void clean(int a)
{
	if(shmdt(mem) == -1)
	{
		printf("%s\n", strerror(errno));
		exit(-1);
	}

	if(fclose(plik) == EOF)
	{
		printf("%s\n", strerror(errno));
		exit(-1);
	}
}

void suma(struct expression e, int wynik[MAXMTX][MAXMTX])
{
	int i, j;
	for(i = 0; i < MAXMTX; ++i)
		for(j = 0; j < MAXMTX; ++j)
			wynik[i][j] = e.mat1[i][j] + e.mat2[i][j];
}

void roznica(struct expression e, int wynik[MAXMTX][MAXMTX])
{
	int i, j;
	for(i = 0; i < MAXMTX; ++i)
		for(j = 0; j < MAXMTX; ++j)
			wynik[i][j] = e.mat1[i][j] - e.mat2[i][j];
}

void iloczyn(struct expression e, int wynik[MAXMTX][MAXMTX])
{
	int i, j, k, d;
	for(i = 0; i < MAXMTX; ++i)
	{
		for(j = 0; j < MAXMTX; ++j)
		{
			d = 0;
			for(k = 0; k < MAXMTX; ++k)
				d += e.mat1[i][k] * e.mat2[k][j];
			wynik[i][j] = d;
		}
	}
}

void wypiszmat(int wynik[MAXMTX][MAXMTX])
{
	char buf[(MAXMTX*MAXMTX+MAXMTX+5)*sizeof(int)];
	buf[0] = '\0';

	int i, j;
	for(i = 0; i < MAXMTX; ++i)
	{
		for(j = 0; j < MAXMTX; ++j)
		{
			sprintf(buf, "%s %i", buf, wynik[i][j]);
		}
		sprintf(buf, "%s%c", buf, '\n');
	}
	sprintf(buf, "%s%s", buf, "\n\0");

	if(fwrite(buf, strlen(buf)*sizeof(char), 1, plik) == EOF)
	{
		printf("%s\n", strerror(errno));
		clean(0);
		exit(-1);
	}
}

void toWrite(struct expression e, int wynik[MAXMTX][MAXMTX])
{
	char tekst[30] = "Wykonalem ";

	switch(e.oper)
	{
		case SUMA:
		{
			suma(e, wynik);
			sprintf(tekst, "%s%s", tekst, "sume");
			break;
		}
		case ROZNICA:
		{
			roznica(e, wynik);
			sprintf(tekst, "%s%s", tekst, "roznice");
			break;
		}
		case ILOCZYN:
		{
			iloczyn(e, wynik);
			sprintf(tekst, "%s%s", tekst, "iloczyn");
			break;
		}
	}

	printf("%s\n", tekst);
	sprintf(tekst, "%s%c", tekst, '\n');

	if(fwrite(tekst, strlen(tekst)*sizeof(char), 1, plik) == EOF)
	{
		printf("%s\n", strerror(errno));
		clean(0);
		exit(-1);
	}

	wypiszmat(e.mat1);
	wypiszmat(e.mat2);
	wypiszmat(wynik);
}

void konsumuj(int ur)
{
	struct expression e;
	int mat3[MAXMTX][MAXMTX];

	struct sembuf sb;
	sb.sem_flg = 0;

	int* begin_k = mem+sizeof(int);
	struct expression* tab = mem+2*sizeof(int);

	if(ur)
		*begin_k = 0;
	int licznik;

	while(1)
	{
		sb.sem_num = KONSUMENT;
		sb.sem_op = ZMNIEJSZ;

		if(semop(sem, &sb, 1) == -1)
		{
			printf("%s\n", strerror(errno));
			clean(0);
			exit(-1);
		}

		sb.sem_num = LICZNIK;

		if(semop(sem, &sb, 1) == -1)
		{
			printf("%s\n", strerror(errno));
			clean(0);
			exit(-1);
		}

		licznik = *begin_k;
		(*begin_k) = (licznik+1)%MAXTAB;

		e = tab[licznik];
		toWrite(e, mat3);

		sb.sem_op = ZWIEKSZ;

		if(semop(sem, &sb, 1) == -1)
		{
			printf("%s\n", strerror(errno));
			clean(0);
			exit(-1);
		}

		sb.sem_num = PRODUCENT;

		if(semop(sem, &sb, 1) == -1)
		{
			printf("%s\n", strerror(errno));
			clean(0);
			exit(-1);
		}
	}
}


int main(int argc, char* argv[])
{
	key_t klucz;
	int ur;

	if(signal(SIGINT, clean) == SIG_ERR)
	{
		printf("%s\n", strerror(errno));
		return -1;
	}

	if((klucz = ftok(KPATH, KVAL)) == -1)
	{
		printf("%s\n", strerror(errno));
		return -1;
	}

	if((sem = semget(klucz, 3, IPC_CREAT | IPC_EXCL | S_IRWXU)) == -1)
	{
		if(errno != EEXIST)
		{
			printf("%s\n", strerror(errno));
			return -1;
		}
		else
		{
			ur = 0;
			if((sem = semget(klucz, 3, 0)) == -1)
			{
				printf("%s\n", strerror(errno));
				return -1;
			}
		}
	}
	else
	{
		if(semctl(sem, PRODUCENT, SETVAL, MAXTAB) == -1)
		{
			printf("%s\n", strerror(errno));
			return -1;
		}

		if(semctl(sem, KONSUMENT, SETVAL, 0) == -1)
		{
			printf("%s\n", strerror(errno));
			return -1;
		}

		if(semctl(sem, LICZNIK, SETVAL, 1) == -1)
		{
			printf("%s\n", strerror(errno));
			return -1;
		}
	}

	if((shm = shmget(klucz, MAXTAB*sizeof(struct expression)+2*sizeof(int), IPC_CREAT | IPC_EXCL | S_IRWXU)) == -1)
	{
		if(errno != EEXIST)
		{
			printf("%s\n", strerror(errno));
			return -1;
		}
		else
		{
			ur = 0;
			if((shm = shmget(klucz, MAXTAB*sizeof(struct expression)+2*sizeof(int), 0)) == -1)
			{
				printf("%s\n", strerror(errno));
				return -1;
			}
		}
	}

	if((mem = shmat(shm, NULL, 0)) == NULL)
	{
		printf("%s\n", strerror(errno));
		return -1;
	}

	char buf[PATH_MAX];
	sprintf(buf, "%s%i", "/tmp/prod", getpid());

	if((plik = fopen(buf, "w+")) == NULL)
	{
		printf("%s\n", strerror(errno));
		return -1;
	}

	konsumuj(ur);

	clean(0);

	return 0;
}
