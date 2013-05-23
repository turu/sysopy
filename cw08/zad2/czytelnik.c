#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#include "types.h"

sem_t* semc;
sem_t* semf;
int shm;
void* mem;
int des;

void clean(int a)
{
	if(munmap(mem, MAXTAB*sizeof(int)+2*sizeof(int)) == -1)
	{
		printf("%s\n", strerror(errno));
		exit(-1);
	}
	
	if(shm_unlink("/shared") == -1)
	{
		printf("%s\n", strerror(errno));
		exit(-1);
	}
	
	if(sem_close(semc) == -1)
	{
		printf("%s\n", strerror(errno));
		exit(-1);
	}
	
	if(sem_close(semf) == -1)
	{
		printf("%s\n", strerror(errno));
		exit(-1);
	}
	
	if(sem_unlink("/czyt") == -1)
	{
		printf("%s\n", strerror(errno));
		exit(-1);
	}
	
	if(sem_unlink("/fo") == -1)
	{
		printf("%s\n", strerror(errno));
		exit(-1);
	}
	
	if(close(des) == -1)
	{
		printf("%s\n", strerror(errno));
		exit(-1);
	}
	
	remove(FPATH);
}

void srednia(int* tab)
{
	int sum = 0;
	int i;
	for(i = 0; i < MAXTAB; ++i)
		sum += tab[i];
		
	printf("Obliczylem srednia: %lf\n", (double)sum/(double)MAXTAB);
}

void maximum(int* tab)
{
	int maks = tab[0];
	int i;
	for(i = 0; i < MAXTAB; ++i)
		if(tab[i] > maks)
			maks = tab[i];
		
	printf("Znalazlem maximum: %i\n", maks);
}

void minimum(int* tab)
{
	int min = tab[0];
	int i;
	for(i = 0; i < MAXTAB; ++i)
		if(tab[i] < min)
			min = tab[i];
		
	printf("Znalazlem minimum: %i\n", min);
}

void pisz(int tryb, int ur)
{
	srand(time(NULL));
	
	int* czyt = mem;
	int* fif = mem+sizeof(int);
	int* tab = mem+2*sizeof(int);
	
	if(ur)
	{
		*czyt = 0;
		*fif = 0;
	}
		
	int pid = getpid();
	int tmp;
	int licznik = 1;
	
	while(1)
	{
		if(write(des, &pid, sizeof(int)) == -1)
		{
			if(errno != EAGAIN)
			{
				printf("%s\n", strerror(errno));
				clean(0);
				exit(-1);
			}
		}
		
		if(sem_wait(semf) == -1)
		{
			printf("%s\n", strerror(errno));
			clean(0);
			exit(-1);
		}

		if(*fif == 0)
		{
			*fif = pid;

			if(sem_post(semf) == -1)
			{
				printf("%s\n", strerror(errno));
				clean(0);
				exit(-1);
			}
		}
		else
		{
			if(*fif == pid)
			{

				if(read(des, &tmp, sizeof(int)) == -1)
				{
					if(errno == EAGAIN)
						*fif = 0;
					else
					{
						printf("%s\n", strerror(errno));
						clean(0);
						exit(-1);
					}	
				}
				else
					*fif = tmp;
				
				if(sem_post(semf) == -1)
				{
					printf("%s\n", strerror(errno));
					clean(0);
					exit(-1);
				}
			
				if(sem_wait(semc) == -1)
				{
					printf("%s\n", strerror(errno));
					clean(0);
					exit(-1);
				}
				
				++(*czyt);
			
				if(sem_post(semc) == -1)
				{
					printf("%s\n", strerror(errno));
					clean(0);
					exit(-1);
				}
				
				switch(tryb)
				{
					case MAXIMUM:
					{
						maximum(tab);
						break;
					}
					case MINIMUM:
					{
						minimum(tab);
						break;
					}
					case SREDNIA:
					{
						srednia(tab);
						break;
					}
				}
				
				printf("Operacja: %i. ", licznik++);
					
				if(sem_wait(semc) == -1)
				{
					printf("%s\n", strerror(errno));
					clean(0);
					exit(-1);
				}
				
				--(*czyt);
			
				if(sem_post(semc) == -1)
				{
					printf("%s\n", strerror(errno));
					clean(0);
					exit(-1);
				}			
			}
			else
			{
				if(sem_post(semf) == -1)
				{
					printf("%s\n", strerror(errno));
					clean(0);
					exit(-1);
				}
			}
		}
	}
}

sem_t* createSemafor(char* path, int* ur)
{
	sem_t* sem;
	if((sem = sem_open(path, O_CREAT | O_EXCL, S_IRWXU, 1)) == SEM_FAILED)
	{
		if(errno != EEXIST)
		{
			printf("%s\n", strerror(errno));
			exit(-1);
		}
		else
		{
			ur = 0;
			if((sem = sem_open(path,  0)) == SEM_FAILED)
			{
				printf("%s\n", strerror(errno));
				exit(-1);
			}
		}
	}
	return sem;
}


int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		printf("Za malo argumentow\n");
		return -1;
	}
	
	int typ;
	
	if(strcmp(argv[1], "MAXIMUM") == 0)
		typ = MAXIMUM;
	else if(strcmp(argv[1], "MINIMUM") == 0)
		typ = MINIMUM;
	else
		typ = SREDNIA;
		
	int ur = 1;
	
	if(signal(SIGINT, clean) == SIG_ERR)
	{
		printf("%s\n", strerror(errno));
		return -1;
	}
	signal(SIGSEGV, clean);
	
	if(mkfifo(FPATH, S_IRWXU) == -1)
	{
		if(errno != EEXIST)
		{
			printf("%s\n", strerror(errno));
			return -1;
		}
	}
	
	if((des = open(FPATH, O_RDWR | O_NONBLOCK)) == -1)
	{
		printf("%s\n", strerror(errno));
		return -1;
	}
	
	semc = createSemafor("/czyt", &ur);
	semf = createSemafor("/fo", &ur);
	
	if((shm = shm_open("/shared", O_RDWR | O_CREAT | O_EXCL, S_IRWXU)) == -1)
	{
		if(errno != EEXIST)
		{
			printf("%s\n", strerror(errno));
			return -1;
		}
		else
		{
			ur = 0;
			if((shm = shm_open("/shared", O_RDWR, 0)) == -1)
			{
				printf("%s\n", strerror(errno));
				return -1;
			}
		}
	}
	
	if(ftruncate(shm, MAXTAB*sizeof(int)+sizeof(int)) == -1)
	{
		printf("%s\n", strerror(errno));
		return -1;
	}
	
	if((mem = mmap(NULL, MAXTAB*sizeof(int)+2*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0)) == NULL)
	{
		printf("%s\n", strerror(errno));
		return -1;
	}
	
	pisz(typ, ur);
	
	clean(0);
	
	return 0;
}
