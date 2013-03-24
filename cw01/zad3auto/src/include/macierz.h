#ifndef _MACIERZ_H_
#define _MACIERZ_H_

#define MAX(a, b) (a) >= (b) ? (a) : (b)
#define MIN(a, b) (a) <= (b) ? (a) : (b)

typedef struct {
    int nw;
    int nk;
    double **d;
} macierz_t;

macierz_t* macierz_alokuj(int nw, int nk);
void macierz_zwolnij(macierz_t *m);

#endif
