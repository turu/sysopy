#ifndef _TYPES_H_
#define _TYPES_H_

#define KPATH "/tmp/serv"
#define KVAL 66

#define MAXMTX 5
#define MAXTAB 10
#define PRODUCENT 0
#define KONSUMENT 1
#define LICZNIK 2
#define ZWIEKSZ 1
#define ZMNIEJSZ -1

typedef struct command {
	double mat[MAXMTX][MAXMTX];
} command;

#endif
