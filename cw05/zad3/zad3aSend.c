#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>

int N;
int counter = 0;

void handler1(int x){
    printf("Odebrano wiadomosc zwrotna\n");
    counter++;
}

void handler2(int x){
    int stat;
    wait(&stat);
    printf("\nW sumie odebrano %d z %d sygnalow\n", counter, N);
    exit(0);
}

int main(int argc, char * argv[]) {
    signal(SIGUSR1, handler1);
    signal(SIGUSR2, handler2);

    if (argc != 2 || (N = atoi(argv[1]))<1 ) {
        N = 2;
        printf("Nie podano ilosci sygnalow! Defaultowo sa %d\n", N);
    }

    int i;
    int PID = fork();

    if (PID == 0){
        if(execve("zad3aRec.out", NULL, NULL) < 0) {
            printf("Exec nie udal sie\n");
        }
    } else {
        i = N;
        while (i--) {
            printf("wyslano SIGUSR1 nr %d\n", N - i);
            kill(PID, SIGUSR1);
            sleep(1);
        }
        printf("wyslano SIGUSR2\n");
        kill(PID, SIGUSR2);

        while(1){
        }
    }

    return 0;
}
