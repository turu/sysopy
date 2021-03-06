#include "commons.h"

sem_t * counter_semaphore;
sem_t * fifo_semaphore;
int operation_ctr = 1;
int target_value;
void * memory;
int shared;
int fifo;

void clean(int v) {
	munmap(memory, MAXTAB*sizeof(int)+2*sizeof(int));
	shm_unlink("/shared");
	sem_close(counter_semaphore);
	sem_close(fifo_semaphore);
	sem_unlink("/readers_ctr");
	sem_unlink("/fifos");
	close(fifo);
	remove(FPATH);
	exit(0);
}

sem_t * getSemaphore(char* path, int* freshly_created) {
	sem_t * sem;

	if ((sem = sem_open(path, O_CREAT | O_EXCL, S_IRWXU, 1)) == SEM_FAILED) {
        freshly_created = 0;
        sem = sem_open(path,  0);
	}

	return sem;
}

void szukoj(int * tab) {
	int  counter = 0;
	int i;
	for(i = 0; i < MAXTAB; ++i)
		if(target_value==tab[i])
            counter++;
	printf("< read number %d >  %d found %d times in the array\n", operation_ctr++, target_value, counter);
}

void czytoj(int freshly_created) {
	srand(time(NULL));
	int * readers_ctr = memory;
	int * next_pid = memory + sizeof(int);
	int * tab = memory + 2 * sizeof(int);

	if (freshly_created) {
		*readers_ctr = 0;
		*next_pid = 0;
	}

	int pid = getpid();
	int tmp;

	while (1) {
		write(fifo, &pid, sizeof(int));

		sem_wait(fifo_semaphore);

		if (*next_pid == 0) {
			*next_pid = pid;
			sem_post(fifo_semaphore);
		} else {
			if (*next_pid == pid) {
				if (read(fifo, &tmp, sizeof(int)) == -1){
                    *next_pid = 0;
				} else {
					*next_pid = tmp;
				}
				sem_post(fifo_semaphore);

				sem_wait(counter_semaphore);
				*readers_ctr++;
				sem_post(counter_semaphore);
                szukoj(tab);
				sem_wait(counter_semaphore);
				*readers_ctr++;
				sem_post(counter_semaphore);
			} else {
				sem_post(fifo_semaphore);
			}
		}
	}
}

int main(int argc, char ** argv) {
    if (argc < 2) {
        printf("Arguments:\n<number> value to look for in the array\n");
        exit(1);
    }

    target_value = atoi(argv[1]);

	int freshly_created = 1;
    signal(SIGINT, clean);
	signal(SIGSEGV, clean);

	mkfifo(FPATH, S_IRWXU);

	fifo = open(FPATH, O_RDWR | O_NONBLOCK);

	counter_semaphore = getSemaphore("/readers_ctr", &freshly_created);
	fifo_semaphore = getSemaphore("/fifos", &freshly_created);

	if ((shared = shm_open("/shared", O_RDWR | O_CREAT | O_EXCL, S_IRWXU)) == -1) {
        freshly_created = 0;
        shared = shm_open("/shared", O_RDWR, 0);
	}

	ftruncate(shared, MAXTAB * sizeof(int) + sizeof(int));
	memory = mmap(NULL, MAXTAB*sizeof(int) + 2 * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shared, 0);

	czytoj(freshly_created);

	clean(0);
	return 0;
}
