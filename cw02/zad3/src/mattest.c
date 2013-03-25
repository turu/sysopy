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

    MyMatrix * I = createIdentity(100, 100);
    printf("Identity of dimension %d x %d created\n", 100, 100);
    checkpoint();
    printMatrix(I);

    MyMatrix * Ipi = mul(I, 3.14);
    printf("I * 3.14=\n");
    printMatrix(Ipi);
    checkpoint();

    MyMatrix * m = createMatrix(100, 100, 7.);
    printf("Yet another matrix created\n");
    printMatrix(m);
    checkpoint();

    MyMatrix * m2 = add(m, Ipi);
    MyMatrix * m3 = sub(m, I);
    MyMatrix * m4 = matmul(m2, m3);
    printf("M4 = (M + I * pi) * (M - I)\n");
    printMatrix(m4);
    checkpoint();

    finalizeMatrix(I);
    finalizeMatrix(Ipi);
    finalizeMatrix(m);
    finalizeMatrix(m2);
    finalizeMatrix(m3);
    finalizeMatrix(m4);
    printf("All matrices destroyed\n");
    checkpoint();

    finalizeMemory();
    printf("Exiting...\n");

    return 0;
}
