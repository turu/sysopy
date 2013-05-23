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

void clean(int a)
{
	if(shmdt(mem) == -1)
	{
		printf("%s\n", strerror(errno));
		exit(-1);
	}
	
	if(shmctl(shm, IPC_RMID, NULL) == -1)
	{
		printf("%s\n", strerror(errno));
		exit(-1);
	}
	
	if(semctl(sem, IPC_RMID, 0) == -1)
	{
		printf("%s\n", strerror(errno));
		exit(-1);
	}
}

void produkuj(int ur)
{
	srand(time(NULL));
	
	struct expression e;
	
	struct sembuf sb;
	sb.sem_flg = 0;
	
	int* begin_p = mem;
	struct expression* tab = mem+2*sizeof(int);
	
	if(ur)
		*begin_p = 0;
	int licznik;
	
	while(1)
	{
		sb.sem_num = PRODUCENT;
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
		
		licznik = *begin_p;
		(*begin_p) = (licznik+1)%MAXTAB;
		
		e.oper = rand()%3;
		int i, j;
		for(i = 0; i < MAXMTX; ++i)
			for(j = 0; j < MAXMTX; ++j)
			{
				e.mat1[i][j] = rand()%10;
				e.mat2[i][j] = rand()%10;
			}
			
		tab[licznik] = e;
		
		printf("Task added to %i cell\n", licznik);	
		
		sb.sem_op = ZWIEKSZ;
		
		if(semop(sem, &sb, 1) == -1)
		{
			printf("%s\n", strerror(errno));
			clean(0);
			exit(-1);
		}
		
		sb.sem_num = KONSUMENT;

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
	int des;
	key_t klucz;
	int ur = 1;
	
	if(signal(SIGINT, clean) == SIG_ERR)
	{
		printf("%s\n", strerror(errno));
		return -1;
	}
	
	if((des = open(KPATH, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR)) == -1)
	{
		printf("%s\n", strerror(errno));
		return -1;
	}
	
	if(close(des) == -1)
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
	
	produkuj(ur);
	
	clean(0);
	
	return 0;
}
