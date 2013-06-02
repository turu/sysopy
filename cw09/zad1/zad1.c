#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>

typedef struct threadInit {
    int fd;
	int thread_count;
	int buf_size;
	char * to_search;
	pthread_t * threads;
	int filesize;
} threadInit;

pthread_mutex_t mutex;
long file_ptr = 0;

void printHelp() {
	printf("Arguments:\n-t <number> number of threads\n-f <filename> path to the file\n-b\
<number> buffer size\n-s <str> pattern to search for\n-h prints help\n");
}

size_t getFileSize(char * filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

void * thread_run(void * args) {
    threadInit * init = (threadInit*) args;
	int fd = init->fd;
	int thread_count = init->thread_count;
	int buf_size = init->buf_size;
	char * to_search = init->to_search;
	pthread_t * threads = init->threads;
	int filesize = init->filesize;

	char * buf = (char *) malloc(sizeof(char) * buf_size);

	int unused = 0;

	#ifdef V_A
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &unused);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &unused);
	#elif V_B
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &unused);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &unused);
	#elif V_C
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &unused);
	#endif

	int i, j = 0;
	int found = 0;
	int pattern_l = strlen(to_search);

	while (1) {
	    pthread_mutex_lock(&mutex);
	    if (file_ptr >= filesize) {
	        break;
	    }
	    int status;
		if ((status = read(fd, buf, buf_size)) < 0) {
		    printf("Failure reading chunk of file at position %ld.\n", file_ptr);
		    exit(19);
		}
		if (status == 0) break;
		buf_size = status;
		file_ptr += buf_size;
		pthread_mutex_unlock(&mutex);

        i = 0;
        while (i < buf_size) {
            j = 0;
            while (i + j < buf_size && j < pattern_l && buf[i + j] == to_search[j]) {
                j++;
            }
            if (j == pattern_l) {
                found = file_ptr + i;
                break;
            }
            i++;
        }

        #ifndef V_C
		if(found) break;
		#endif
		#ifdef V_B
		pthread_testcancel();   //setting cancellation point
		#endif
	}

	if (found) {
		printf("TID = %d\t file_ptr=%d\n", (int)pthread_self(), found - buf_size);

        #ifndef V_C
		for (i = 0; i < thread_count; i++) {
			if (pthread_self() != threads[i]) {
				pthread_cancel(threads[i]);
			}
		}
        #endif
	} else {
        printf("TID = %d not found any occurences of the pattern.\n", (int)pthread_self());
	}

	return NULL;
}


int main(int argc, char ** argv) {
	int thread_count = 1;
	int buf_size = 256;
	char * filename = malloc(sizeof(char) * 256);
	char * to_search = malloc(sizeof(char) * 256);
	int args_passed = 0;

	int c;
	while((c = getopt(argc, argv, "t:f:b:s:h")) != -1) {
		switch (c) {
			case 't':
                thread_count = atoi(optarg);
                args_passed++;
                break;
            case 'f':
                filename = strcpy(filename, optarg);
                args_passed++;
                break;
			case 'b':
                buf_size = atoi(optarg);
                args_passed++;
                break;
            case 's':
                to_search = strcpy(to_search, optarg);
                args_passed++;
                break;
			case 'h':
                printHelp();
                exit(0);
                break;
			default:
                printHelp();
                exit(1);
		}
	}

	if (args_passed != 4) {
	    printHelp();
	    exit(21);
	}

	int fd = open(filename, O_RDONLY);
	if (fd < 0) {
		printf("Could not open file\n");
		exit(2);
	}

	if(pthread_mutex_init(&mutex, NULL) < 0) {
	    printf("Could not open estabilish a mutex\n");
		exit(3);
	}

	pthread_t * threads = (pthread_t *) malloc(sizeof(pthread_t) * thread_count);

	threadInit * args = (threadInit*) malloc(sizeof(threadInit));
	args->fd = fd;
	args->thread_count = thread_count;
	args->buf_size = buf_size;
	args->to_search = to_search;
	args->threads = threads;
	args->filesize = (int) getFileSize(filename);

	int i;
	for (i = 0; i < thread_count; i++) {
		if(pthread_create(&threads[i], NULL, &thread_run, (void *)args) < 0) {
		    printf("Could not create thread no %d.\n", i);
		    exit(4);
		}
		printf("Created thread id %d.\n", (int) threads[i]);
	}

	for(i = 0 ; i < thread_count ; i++)
		if(pthread_join(threads[i], NULL) < 0) {
			printf("Could not join thread no %d to the main one.\n", i);
		    exit(5);
		}

	if(close(fd) < 0) {
        printf("Could not close file.\n");
        exit(6);
	}

	if(pthread_mutex_destroy(&mutex) < 0) {
        printf("Failure closing mutex\n");
        exit(7);
	}

	free(to_search);
	free(filename);

	return 0;
}
