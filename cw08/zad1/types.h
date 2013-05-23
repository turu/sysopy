#ifndef TYPES_H
#define TYPES_H

#define KPATH "/tmp/serv"
#define KVAL 66

#define SUMA 0
#define ROZNICA 1
#define ILOCZYN 2
#define MAXMTX 5
#define MAXTAB 10
#define PRODUCENT 0
#define KONSUMENT 1
#define LICZNIK 2
#define ZWIEKSZ 1
#define ZMNIEJSZ -1

struct expression
{
	int oper;
	int mat1[MAXMTX][MAXMTX];
	int mat2[MAXMTX][MAXMTX];
};

#endif
