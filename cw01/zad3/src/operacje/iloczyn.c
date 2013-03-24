#include <stdio.h>

#include "suma.h"
#include "macierz.h"

macierz_t *iloczyn(macierz_t *m1, macierz_t *m2) {

#ifdef KOMUNIKATY
	fprintf(stderr, ">>> funkcja iloczyn\n");
#endif

    int nw = m1->nw;
    int nk = m2->nk;
    
    int nn = MIN(m1->nk, m2->nw);
        
    macierz_t *mw = macierz_alokuj(nw, nk);
    
    int w, k, n;
    
    for(w = 0; w < nw; w++)
        for(k = 0; k < nk; k++)
            for(n = 0; n < nn; n++)
                mw->d[w][k] += m1->d[w][n] * m1->d[n][k];
    
    return mw;
}
