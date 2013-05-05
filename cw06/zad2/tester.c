#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int isPrime(int number) {
    if (number <= 1) return 0;
    unsigned int i;
    for (i=2; i*i<=number; i++) {
        if (number % i == 0) return 0;
    }
    return 1;
}

int main (int argc, char **argv){
    if (argc < 2) {
        printf("Bo zupa byla za slona.\n");
        return 1;
    }

    char * path = argv[1];
    int fd = open(path, O_RDONLY | O_NONBLOCK);
    char buff[128];
    int rst;

    while (1) {
        rst = read(fd, buff, sizeof(buff));
        if (!rst) {
            sleep(1);
        } else {
            char * ptr = strtok(buff, "\n"), * ptr2;
            ptr2 = strtok(NULL, "\n");

            while (ptr2 != NULL) {
                int number = atoi(ptr);
                if (isPrime(number)) {
                    printf("Znalazle prime = %d\n", number);
                } else {
                    printf("Try harder - %d\n", number);
                }
                ptr = ptr2;
                ptr2 = strtok(NULL, "\n");
            }
        }
    }

    return 0;
}
