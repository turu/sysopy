#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int main() {
	double d = 0;
	int i = 0;
	for(i = 0 ; i < 100000000; i++) {
		d = sin(i);
		d = d * log(7 * d);
		d = sqrt(d);
		d = d + 3 * d;
		d = d * d - 1;
		d = cos(i) * d;
	}

	return 0;
}
