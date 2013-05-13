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

void successHandler() {
    printf("Message persisted.\n");
}

void failureHandler() {
    printf("Message not persisted.\n");
}

void randomChars(char * data, size_t size) {
    while (size--) {
        *data = (char) (rand() % 95 + 32);
        data++;
    }
    *data = '\0';
}

int main(int argc, char * argv[]){
    srand(time(NULL));

    if (argc < 2) {
        printf("Podaj swoje hiper-unikalne userid\n");
        return 1;
    }

    struct mq_attr attr;
    attr.mq_maxmsg = HSQ_SIZE;
    attr.mq_msgsize = sizeof(message);
    attr.mq_flags = 0;
    mqd_t serverQueue = mq_open (SERVERQUEUE, O_WRONLY, 0664, &attr);
    if (serverQueue < 0) {
        printf("Could not open server connection.\n");
        return 2;
    }
    printf("Connected to server, id: %d\n", serverQueue);

    message msg;
    msg.pid = getpid();
    sprintf(msg.name, "%s", argv[1]);

    signal(SIGUSR1, successHandler);
    signal(SIGUSR2, failureHandler);

    while (1) {
        sleep(1);
        randomChars(&msg.content, rand() % 128);
        int rc = mq_send(serverQueue, (char*)(&msg), sizeof(msg), 0);

        if (rc < 0) {
            printf("Failure sending message. Check connection.\n");
            return 2;
        }
        printf("Message sent...\n");
    }

    return 0;
}
