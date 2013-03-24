#include <stdio.h>

int func(int n){
    return n;
}


int main() {

    int l = 0;
    int i;
	for(i = 0; i < 2000000000; i++) {
		l = func(i + l);
	}

	return 0;
}
