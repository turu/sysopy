#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "macierz.h"
#include "wczytaj.h"
#include "wypisz.h"
#include "suma.h"
#include "roznica.h"
#include "iloczyn.h"
#include <config.h>

#ifdef WEWY_PLIKI
#include "pliki.h"
#endif

int main(int argc, char *argv[]) {

#ifdef KOMUNIKATY
#ifdef WEWY_PLIKI
    fprintf(stderr, "Program: plikowe we/wy\n");
#else
    fprintf(stderr, "Program: konsolowe we/wy\n");
#endif
#endif

#ifdef WEWY_PLIKI
    if(argc < 4) {
        printf("brakujacy argumenty: operacja macierz_A macierz_B\n\n");
        return 1;
    }
    USTAL_PLIK(argv[2]);
    macierz_t *m1 = wczytaj();
    USTAL_PLIK(argv[3]);
    macierz_t *m2 = wczytaj();
#else
    if(argc < 2) {
        printf("brakujacy argument (operacja): suma | roznica | iloczyn\n\n");
        return 1;
    }
    macierz_t *m1 = wczytaj();
    macierz_t *m2 = wczytaj();
#endif

    macierz_t *m_w = NULL;
    if(strcmp(argv[1], "suma") == 0) {
        m_w = suma(m1, m2);
    } else if(strcmp(argv[1], "roznica") == 0) {
        m_w = roznica(m1, m2);
    } else if(strcmp(argv[1], "iloczyn") == 0) {
        m_w = iloczyn(m1, m2);
    } else {
        printf("niepoprawna nazwa operacji\n\n");
    }

    if(m_w) {
#ifdef WEWY_PLIKI
        USTAL_PLIK("wynik");
        wypisz(m_w);
#else
        printf("Macierz A :\n");
        wypisz(m1);
        printf("Macierz B :\n");
        wypisz(m2);
        printf("%s ( A , B ) :\n", argv[1]);
        wypisz(m_w);
#endif
        macierz_zwolnij(m_w);
    }

    macierz_zwolnij(m1);
    macierz_zwolnij(m2);

    return 0;
}
