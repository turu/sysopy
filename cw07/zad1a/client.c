#include <sys/types.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/times.h>
#include <mqueue.h>
#include "commons.h"

int pid;

void clean(){
    kill(pid, SIGKILL);
    exit(0);
}

int main(int argc, char * argv[]){
    if (argc < 2) {
        printf("Podaj swoje hiper-unikalne userid\n");
        return 1;
    }

    int handShakeQueue = msgget(ftok(HANDSHAKEQUEUE, 1), 0);

    printf("Estabilished connection with server.\n");
    char * name = argv[1];
    idMsg myId;
    myId.mtype = CLIENT;
    sprintf(myId.id, "%s", name);

    int rc = msgsnd(handShakeQueue, &myId, sizeof(myId.id), 0);
    sleep(1);

    if (rc == -1) {
        printf("Could not log onto a server %s. Try again later.\n", HANDSHAKEQUEUE);
        return 1;
    }

    message msg;

    msgrcv(handShakeQueue, &myId, sizeof(myId.id), SERVER, 0);
    int recQueue = atoi(myId.id);
    msgrcv(handShakeQueue, &myId, sizeof(myId.id), SERVER, 0);
    int sendQueue = atoi(myId.id);

    printf("rec=%d send=%d\n", recQueue, sendQueue);

    printf("Login successful.\n");

    pid = fork();
    atexit(clean);
    signal(SIGINT, clean);

    if (pid != 0) {
        while (1) {
            printf("To >>>> ");
            fgets(msg.to, sizeof(msg.to), stdin);
            printf("What >> ");
            fgets(msg.content, sizeof(msg.content), stdin);
            msg.time = times(NULL);
            msg.mtype = CLIENT;
            int rc = msgsnd(sendQueue, &msg, sizeof(msg.time) + sizeof(msg.to) + sizeof(msg.content), 0);

            if (rc == -1) {
                printf("Failure sending message. Check connection.\n");
            }
        }
    }

    while (1) {
        sleep(1);
        rc = msgrcv(recQueue, &msg, sizeof(msg.time) + sizeof(msg.to) + sizeof(msg.content), SERVER, 0);

        if (rc != -1) {
            printf("\n------------------------------\nMessage from: %s\nSent at: %s\n%s", msg.to, ctime(&msg.time), msg.content);
        }
    }

    return 0;
}
