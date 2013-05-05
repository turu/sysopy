#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define WRITE 1
#define READ 0

int main (int argc, char ** argv) {
    if (argc < 2) {
        printf("Nie podales pliku do przetworzenia. Zegnam ozieble.\n");
        return 1;
    }

    pid_t pid;
    int fd[2];
    int fd2[2];

    pipe(fd);
    pipe(fd2);
    pid = fork();

    if (pid == 0) {
        close(fd[READ]);
        dup2(fd[WRITE], STDOUT_FILENO);
        execlp("grep", "grep", "Ala", argv[1], NULL);
    } else {
        pid = fork();
        if (pid == 0) {
            close(fd[WRITE]);
            dup2(fd[READ], STDIN_FILENO);
            close(fd2[READ]);
            dup2(fd2[WRITE], STDOUT_FILENO);
            execlp("grep", "grep", "-i", "kot", NULL);
        }

        close(fd[WRITE]);
        close(fd2[WRITE]);
        dup2(fd2[READ], STDIN_FILENO);
        execlp("wc", "wc","-l",NULL);
    }

    return 0;
}
