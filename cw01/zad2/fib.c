#include <stdio.h>
#include <stdlib.h>

int fib(int n) {
	if (n==0) return 0;
	if (n==1) return 1;
	else return fib(n-1) + fib(n-2);
}

int main(int argc, char ** argv) {
	int n;
	n = atoi(argv[1]);
	printf("%d\n", fib(n));
	return 0;
}
