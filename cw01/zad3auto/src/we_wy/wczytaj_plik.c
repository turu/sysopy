#include <stdio.h>

#include "wczytaj.h"
#include "macierz.h"
#include "pliki.h"
#include <config.h>

macierz_t *wczytaj() {

#ifdef KOMUNIKATY
	fprintf(stderr, ">>> funkcja wczytaj (plik)\n");
#endif

    FILE *plik = fopen(NAZWA_PLIKU, "r");
    if(plik == NULL)
        return NULL;

    int nw, nk;
    fscanf(plik, "%d %d", &nw, &nk);

    macierz_t *mw = macierz_alokuj(nw, nk);

    int w, k;
    for(w = 0; w < nw; w++)
        for(k = 0; k < nk; k++)
            fscanf(plik, "%lf", &mw->d[w][k]);

    fclose(plik);

    return mw;
}
