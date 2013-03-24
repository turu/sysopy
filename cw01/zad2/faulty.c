#include <stdio.h>
#include <stdlib.h>

const int MAGIC = 7;
const int iterations = 10;

void func2(int * t, int i) {
    int act = t[i];
    t[i] = i * MAGIC + act;
    t[i] /= i - iterations;
}

void func1(int * t, int s) {
    int i;
    for(i = 0; i <= s; i++) {
        func2(t, i);
    }
}

int main() {

    int * a = (int*) malloc(iterations * sizeof(int));

	int i;
	for(i = 0; i <= iterations; i++) {
        func1(a, i);
	}

    free(a);

	return 0;
}

