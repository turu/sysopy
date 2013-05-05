#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <wait.h>

int counter = 0;

void func_usr1(int x) {
    printf(" Odebralem sygnal\n");
    counter++;
}

void func_usr2(int x) {
    printf(" W sumie odebralem %d sygnalow\n", counter);
    int parentPID = getppid();
    int l = counter;

    while (l--) {
        printf(" Przesylam sygnal spowrotem\n");
        kill(parentPID, SIGRTMAX);
        sleep(2);
    }

    printf(" Przesylam potwierdzenie odebrania sygnalu\n");
    kill(parentPID, SIGRTMIN);

    exit(counter);
}

int main() {
    signal(SIGRTMAX, func_usr1);
    signal(SIGRTMIN, func_usr2);

    while(1) {
    }

    return 0;
}
