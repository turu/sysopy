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

int serverQueue;
int maxSize;
FILE * out;

void closeQueue(char * name, int queue) {
    msgctl(queue, IPC_RMID, NULL);
	unlink(name);
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
    serverQueue = createQueue(SERVERQUEUE);
    printf("Server queue created. Server running...\n");

    //atexit(cleanUp);
    signal(SIGINT, cleanUp);

    int rec_Status;
    message msg;
    text reply;

    while (1) {
        sleep(1);

        rec_Status = msgrcv(serverQueue, &msg, sizeof(msg), CLIENT, IPC_NOWAIT);
        if (rec_Status >= 0) {
            printf("-----------------------------------\n");
            printf("Received message from pid = %d\n", msg.pid);
            printf("Name = %s\tContent = \n%s\n", msg.name, msg.content);
            reply.mtype = msg.pid;
            if (save(&msg) == -1) {
                printf("Message not persisted.\n");
                sprintf(reply.text, "Could not save to file. Go to hell.");
            } else {
                printf("Message persisted.\n");
                sprintf(reply.text, "Message persisted correctly");
            }

            rec_Status = msgsnd(serverQueue, &reply, sizeof(reply), 0);

            if (rec_Status < 0) {
                printf("Reply could not be sent.\n");
            }
        }
    }

    return 0;
}
