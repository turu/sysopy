#include <stdio.h>

#include "wczytaj.h"
#include "macierz.h"
#include <config.h>

macierz_t *wczytaj() {

#ifdef KOMUNIKATY
	fprintf(stderr, ">>> funkcja wczytaj (konsola)\n");
#endif

    int nw, nk;
    printf("Podaj wymiary macierzy (np. 5 4): ");
    scanf("%d %d", &nw, &nk);

    macierz_t *mw = macierz_alokuj(nw, nk);

    int w, k;
    for(w = 0; w < nw; w++)
        for(k = 0; k < nk; k++) {
            printf("M[%d, %d] = ", w + 1, k + 1);
            scanf("%lf", &mw->d[w][k]);
        }

    return mw;
}
