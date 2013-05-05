#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main (int argc, char **argv){
    FILE * pipe = popen("sort", "w");
    char buffer[128];
    char * status = gets(buffer);

    //wait for ctrl+d
    while (status != NULL) {
        fprintf(pipe, "%s\n", buffer);
        status = gets(buffer);
    }
    printf("Wejscie weszlo, tera sortujemy.\n\n");

    return 0;
}
