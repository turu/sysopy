#include <stdio.h>

#include "suma.h"
#include "macierz.h"
#include <config.h>

macierz_t *suma(macierz_t *m1, macierz_t *m2) {

#ifdef KOMUNIKATY
	fprintf(stderr, ">>> funkcja suma\n");
#endif

    int nw_max = MAX(m1->nw, m2->nw);
    int nk_max = MAX(m1->nk, m2->nk);

    macierz_t *mw = macierz_alokuj(nw_max, nk_max);

    int w, k;

    for(w = 0; w < m1->nw; w++)
        for(k = 0; k < m1->nk; k++)
            mw->d[w][k] = m1->d[w][k];

    for(w = 0; w < m2->nw; w++)
        for(k = 0; k < m2->nk; k++)
            mw->d[w][k] += m2->d[w][k];


    return mw;
}
