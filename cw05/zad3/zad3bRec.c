#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <wait.h>

int counter = 0;

void func_usr1(int x) {
    printf(" Odebralem sygnal\nWysylam potwierdzenie\n");
    kill(getppid(), SIGUSR1);
    counter++;
}

void func_usr2(int x) {
    printf(" W sumie odebralem %d sygnalow\n", counter);
    int parentPID = getppid();

    printf(" Przesylam potwierdzenie odebrania sygnalu\n");
    kill(parentPID, SIGUSR2);

    exit(counter);
}

int main() {
    signal(SIGUSR1, func_usr1);
    signal(SIGUSR2, func_usr2);

    while(1) {
    }

    return 0;
}
