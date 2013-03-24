#ifndef _PLIKI_H_
#define _PLIKI_H_

#include <string.h>

extern char __plik[512];

#define USTAL_PLIK(a) strcpy(__plik, (a))
#define NAZWA_PLIKU (__plik)

#endif
