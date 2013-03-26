#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "mymat.h"
#include "execution.h"

#ifdef DLL
#include <dlfcn.h>
#endif

int main() {
    #ifdef DLL
    void * handle2;
	handle2 = dlopen("lib/libmymem.so", RTLD_LAZY);
	if(handle2 == NULL) {
		fprintf(stderr, "Nieudane zaladowanie biblioteki libmymem.so\n");
		return -1;
	}

    void (*memInit)(int) = dlsym(handle2, "memInit");
    void (*finalizeMemory)() = dlsym(handle2, "finalizeMemory");
    void * (*mylloc)(size_t) = dlsym(handle2, "mylloc");
    int (*myfree)(void*) = dlsym(handle2, "myfree");
    MyStatus * (*getMyStatus)() = dlsym(handle2, "getMyStatus");

	void * handle;
	handle = dlopen("lib/libmymat.so", RTLD_LAZY);
	if(handle == NULL) {
		fprintf(stderr, "Nieudane zaladowanie biblioteki libmymat.so\n");
		return -1;
	}

	MyMatrix * (*createMatrixNoInit)(unsigned int, unsigned int) = dlsym(handle, "createMatrixNoInit");
    MyMatrix * (*createMatrix)(unsigned int, unsigned int, const double) = dlsym(handle, "createMatrix");
    MyMatrix * (*createIdentity)(unsigned int, unsigned int) = dlsym(handle, "createIdentity");
    void (*finalizeMatrix)(MyMatrix *) = dlsym(handle, "finalizeMatrix");
    void (*inc)(MyMatrix *, MyMatrix *) = dlsym(handle, "inc");
    MyMatrix * (*add)(MyMatrix *, MyMatrix *) = dlsym(handle, "add");
    void (*dec)(MyMatrix *, MyMatrix *) = dlsym(handle, "dec");
    MyMatrix * (*sub)(MyMatrix *, MyMatrix *) = dlsym(handle, "sub");
    MyMatrix * (*matmul)(MyMatrix *, MyMatrix *) = dlsym(handle, "matmul");
    MyMatrix * (*mul)(MyMatrix *, const double) = dlsym(handle, "mul");
    void (*printMatrix)(MyMatrix *) = dlsym(handle, "printMatrix");
	#endif

    printf("Execution started\n");
    checkpoint();
    #ifdef DLL
    printMemStatus(getMyStatus());
    #endif

    MyMatrix * I = createIdentity(10, 10);
    printf("\nIdentity of dimension %d x %d created\n", 10, 10);
    printMatrix(I);
    checkpoint();
    #ifdef DLL
    printMemStatus(getMyStatus());
    #endif

    MyMatrix * Ipi = mul(I, 3.14);
    printf("\nI * 3.14=\n");
    printMatrix(Ipi);
    checkpoint();
    #ifdef DLL
    printMemStatus(getMyStatus());
    #endif

    MyMatrix * m = createMatrix(10, 10, 7.);
    printf("\nYet another matrix created, M = \n");
    printMatrix(m);
    checkpoint();
    #ifdef DLL
    printMemStatus(getMyStatus());
    #endif

    MyMatrix * m2 = add(m, Ipi);
    printf("\nM2 = M + Ipi\n");
    printMatrix(m2);
    checkpoint();
    #ifdef DLL
    printMemStatus(getMyStatus());
    #endif

    MyMatrix * m3 = sub(m, I);
    printf("\nM3 = M - I\n");
    printMatrix(m3);
    checkpoint();
    #ifdef DLL
    printMemStatus(getMyStatus());
    #endif

    MyMatrix * m4 = matmul(m2, m3);
    printf("\nM4 = M2 * M3\n");
    printMatrix(m4);
    checkpoint();
    #ifdef DLL
    printMemStatus(getMyStatus());
    #endif

    finalizeMatrix(I);
    finalizeMatrix(m);
    finalizeMatrix(m2);
    finalizeMatrix(m3);
    finalizeMatrix(m4);
    printf("All matrices destroyed\n");
    checkpoint();
    #ifdef DLL
    printMemStatus(getMyStatus());
    #endif

    finalizeMemory();
    printf("Exiting...\n");

    return 0;
}
