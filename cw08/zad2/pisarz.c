#include "commons.h"

sem_t * counter_semaphore;
sem_t * fifo_semaphore;
int shared;
void * memory;
int fifo;

void clean(int v) {
	munmap(memory, MAXTAB*sizeof(int)+2*sizeof(int));
	shm_unlink("/shared");
	sem_close(counter_semaphore);
    sem_close(fifo_semaphore);
	sem_unlink("/readers_counter");
	sem_unlink("/fifos");
	close(fifo);
	remove(FPATH);
	exit(0);
}

void pisoj(int freshly_created){
	srand(time(NULL));

	int * readers_counter = memory;
	int * operating_next_pid = memory + sizeof(int);
	int * tab = memory + 2 * sizeof(int);

	if (freshly_created) {
		*readers_counter = 0;
		*operating_next_pid = 0;
	}

	int pid = getpid();
	int tmp;
	int write_number = 1;

	while (1) {
		write(fifo, &pid, sizeof(int));

        sem_wait(fifo_semaphore);

		if (*operating_next_pid == 0) {
			sem_post(fifo_semaphore);
			*operating_next_pid = pid;
		} else {
			if (*operating_next_pid == pid) {
				if (read(fifo, &tmp, sizeof(int)) == -1) {
					if(errno == EAGAIN){
						*operating_next_pid = 0;
					}
				} else {
					*operating_next_pid = tmp;
				}
				sem_post(fifo_semaphore);

				while (1) {
					sem_wait(counter_semaphore);

					if(*readers_counter == 0)
						break;

					sem_post(counter_semaphore);
				}

                int how_many = rand() % MAXTAB;
                int i = 0, k = how_many;
                while (how_many--) {
                    i+=rand()%(MAXTAB-i-how_many);
                    tab[i]=rand()%15;
                }

				printf("(%d) %d values had been modified\n", write_number++, k);

				sem_post(counter_semaphore);
			} else {
				sem_post(fifo_semaphore);
			}
		}
	}
}

sem_t * getSemaphore(char* path, int* freshly_created) {
	sem_t * sem;

	if ((sem = sem_open(path, O_CREAT | O_EXCL, S_IRWXU, 1)) == SEM_FAILED) {
        freshly_created = 0;
        sem = sem_open(path,  0);
	}

	return sem;
}


int main(int argc, char ** argv) {
	int freshly_created = 1;
	signal(SIGINT, clean);
	signal(SIGSEGV, clean);

	mkfifo(FPATH, S_IRWXU);
	fifo = open(FPATH, O_RDWR | O_NONBLOCK);

	counter_semaphore = getSemaphore("/readers_counter", &freshly_created);
	fifo_semaphore = getSemaphore("/fifos", &freshly_created);

	if((shared = shm_open("/shared", O_RDWR | O_CREAT | O_EXCL, S_IRWXU)) == -1){
        freshly_created = 0;
        shared = shm_open("/shared", O_RDWR, 0);
	}

	ftruncate(shared, MAXTAB*sizeof(int)+sizeof(int));
	memory = mmap(NULL, MAXTAB*sizeof(int)+2*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shared, 0);

	pisoj(freshly_created);

	clean(0);
	return 0;
}
