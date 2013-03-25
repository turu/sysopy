#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "mymat.h"
#include "execution.h"

#ifdef DLL
#include <dlfnc.h>
#endif

int main() {
    printf("Execution started\n");
    checkpoint();



    return 0;
}
