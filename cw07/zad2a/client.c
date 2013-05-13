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

    int serverQueue = msgget(ftok(SERVERQUEUE, 1), 0);

    message msg;
    msg.mtype = CLIENT;
    msg.pid = getpid();
    sprintf(msg.name, "%s", argv[1]);
    text reply;

    while (1) {
        sleep(1);
        randomChars(&msg.content, rand() % 128);
        int rc = msgsnd(serverQueue, &msg, sizeof(msg), 0);

        if (rc < 0) {
            printf("Failure sending message. Check connection.\n");
            return 2;
        }
        printf("Message sent...\n");

        rc = msgrcv(serverQueue, &reply, sizeof(reply), msg.pid, 0);

        if (rc < 1) {
            printf("Could not receive server response.\n");
            return 3;
        }

        printf("Server's response: %s\n", reply.text);
    }

    return 0;
}
