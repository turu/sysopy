#ifndef _COMMONS_H_
#define _COMMONS_H_

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

#define KPATH "/tmp/serv"
#define KVAL 66

#define MAXMTX 5
#define MAXTAB 10
#define PRODUCENT 0
#define CONSUMER 1
#define CTR 2
#define INC 1
#define DEC -1

typedef struct command {
	double mat[MAXMTX][MAXMTX];
} command;

#endif
