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
    kill(pid,SIGKILL);
    exit(0);
}

int main(int argc, char * argv[]){
    if (argc < 2) {
        printf("Podaj swoje hiper-unikalne userid\n");
        return 1;
    }

    struct mq_attr attr;
    attr.mq_maxmsg = HSQ_SIZE;
    attr.mq_msgsize = sizeof(char)*128;
    attr.mq_flags = 0;
    mqd_t handShakeQueue = mq_open (HANDSHAKEQUEUE, O_WRONLY, 0664, &attr);
    if (handShakeQueue < 0) {
        printf("Could not open server connection.\n");
        return 2;
    }

    printf("Estabilished connection with server (id: %d).\n", handShakeQueue);
    char * name = argv[1];

    int rc = mq_send(handShakeQueue, name, sizeof(char) * 128, 0);
    sleep(1);

    if (rc == -1) {
        printf("Could not log onto a server %s. Try again later.\n", HANDSHAKEQUEUE);
        return 1;
    }

    message msg;

    char buff[128];
    sprintf(buff, "/%s", name);
    mqd_t recQueue = createQueue(buff, sizeof(msg));
    sprintf(buff, "/%sr", name);
    mqd_t sendQueue = createQueue(buff, sizeof(msg));

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
            int rc = mq_send(sendQueue, (char*)(&msg), sizeof(msg) , 0);

            if (rc == -1) {
                printf("Failure sending message. Check connection.\n");
            }
        }
    }

    while (1) {
        sleep(1);
        rc = mq_receive(recQueue, (char*)(&msg), sizeof(msg), NULL);

        if (rc != -1) {
            printf("\n------------------------------\nMessage from: %s\nSent at: %s\n%s", msg.to, ctime(&msg.time), msg.content);
        }
    }

    return 0;
}
