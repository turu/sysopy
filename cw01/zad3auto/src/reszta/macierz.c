#include <stdlib.h>
#include <stdio.h>

#include "macierz.h"
#include <config.h>

macierz_t* macierz_alokuj(int nw, int nk) {

#ifdef KOMUNIKATY
	fprintf(stderr, ">>> funkcja macierz_alokuj\n");
#endif

    macierz_t *m = malloc(sizeof(macierz_t));

    m->nw = nw;
    m->nk = nk;

    double *buff = malloc(sizeof(double) * nw * nk);
    m->d = malloc(sizeof(double *) * nw);
    int i, j;
    for(i = 0; i < nw; i++)
        m->d[i] = buff + i * nk;

    for(i = 0; i < nw; i++)
        for(j = 0; j < nk; j++)
            m->d[i][j] = 0;

    return m;
}

void macierz_zwolnij(macierz_t *m) {

#ifdef KOMUNIKATY
	fprintf(stderr, ">>> funkcja macierz_zwolnij\n");
#endif

    free(m->d[0]);
    free(m->d);
}
