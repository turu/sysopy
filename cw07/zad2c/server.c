#include <sys/stat.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <mqueue.h>
#include "commons.h"

mqd_t serverQueue;
int maxSize;
FILE * out;

void closeQueue(char * name, mqd_t queue) {
    mq_close(queue);
	mq_unlink(name);
}

void cleanUp() {
    closeQueue(SERVERQUEUE, serverQueue);
    fclose(out);

    exit(0);
}

int save(message * msg) {
    int contSize = strlen(msg->content);
    if (contSize > maxSize)
        return -1;
    time_t curTime = time(NULL);
    char timeStr[30];
    sprintf(timeStr, "%s", ctime(&curTime));

    fprintf(out, "------------------------------------------------------------\n>>> Message:\nTime: %s\nUser: %s\nContent: %s\n",
            timeStr, msg->name, msg->content);

    maxSize -= contSize;

    return 1;
}

int main(int argc, char ** argv) {
    if (argc < 2) {
        printf("Nie podano max rozmiaru pliku!\n");
        return 1;
    }

    maxSize = atoi(argv[1]);
    out = fopen("log.log", "w");
    serverQueue = createQueue(SERVERQUEUE, sizeof(message));
    if (serverQueue < 0) {
        printf("Failed creating server queue.\n");
        return 2;
    }
    printf("Server queue (id: %d) created. Server running...\n", serverQueue);

    //atexit(cleanUp);
    signal(SIGINT, cleanUp);

    int rec_Status;
    message msg;

    while (1) {
        sleep(1);

        rec_Status = mq_receive(serverQueue, (char*)(&msg), sizeof(msg), NULL);
        if (rec_Status != -1) {
            printf("-----------------------------------\n");
            printf("Received message from pid = %d\n", msg.pid);
            printf("Name = %s\tContent = \n%s\n", msg.name, msg.content);
            if (save(&msg) == -1) {
                printf("Message not persisted.\n");
                kill(msg.pid, SIGUSR2);
            } else {
                printf("Message persisted.\n");
                kill(msg.pid, SIGUSR1);
            }
        }
    }

    return 0;
}
