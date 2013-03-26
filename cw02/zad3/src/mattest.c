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

    MyMatrix * I = createIdentity(10, 10);
    printf("\nIdentity of dimension %d x %d created\n", 10, 10);
    printMatrix(I);
    checkpoint();

    MyMatrix * Ipi = mul(I, 3.14);
    printf("\nI * 3.14=\n");
    printMatrix(Ipi);
    checkpoint();

    MyMatrix * m = createMatrix(10, 10, 7.);
    printf("\nYet another matrix created, M = \n");
    printMatrix(m);
    checkpoint();

    MyMatrix * m2 = add(m, Ipi);
    printf("\nM2 = M + Ipi\n");
    printMatrix(m2);
    checkpoint();

    MyMatrix * m3 = sub(m, I);
    printf("\nM3 = M - I\n");
    printMatrix(m3);
    checkpoint();

    MyMatrix * m4 = matmul(m2, m3);
    printf("\nM4 = M2 * M3\n");
    printMatrix(m4);
    checkpoint();

    finalizeMatrix(I);
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
