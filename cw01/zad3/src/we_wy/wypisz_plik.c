#include <stdio.h>

#include "wczytaj.h"
#include "macierz.h"
#include "pliki.h"

void wypisz(macierz_t *m) {

#ifdef KOMUNIKATY
	fprintf(stderr, ">>> funkcja wypisz (plik)\n");
#endif
    
    FILE *plik = fopen(NAZWA_PLIKU, "w");
    if(plik == NULL)
        return;
    
    fprintf(plik, "%d %d\n", m->nw, m->nk);
    
    int w, k;
    for(w = 0; w < m->nw; w++) {
        for(k = 0; k < m->nk; k++)
            fprintf(plik, "\t%f", m->d[w][k]);
        fprintf(plik, "\n");
    }
    
    fclose(plik);
}
