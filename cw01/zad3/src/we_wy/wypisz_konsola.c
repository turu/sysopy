#include <stdio.h>

#include "wczytaj.h"
#include "macierz.h"

void wypisz(macierz_t *m) {

#ifdef KOMUNIKATY
	fprintf(stderr, ">>> funkcja wypisz (konsola)\n");
#endif

    printf("%d %d\n", m->nw, m->nk);
    
    int w, k;
    for(w = 0; w < m->nw; w++) {
        for(k = 0; k < m->nk; k++)
            printf("\t%f", m->d[w][k]);
        printf("\n");
    }
}
