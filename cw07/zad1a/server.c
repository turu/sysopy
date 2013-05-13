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

int handShakeQueue;
int * sendQueues;
int * recQueues;
char ** clientIDs;
int sendQueues_size = 0;

void closeQueue(char * name, int queue) {
    msgctl(queue, IPC_RMID, NULL);
	unlink(name);
}

void cleanUp() {
    closeQueue(HANDSHAKEQUEUE, handShakeQueue);
    int i;
    for (i = 0; i < sendQueues_size; i++) {
        closeQueue(clientIDs[i],sendQueues[i]);
        free(clientIDs[i]);
    }

    free(sendQueues);
    free(recQueues);
    free(clientIDs);

    printf("\n<%d>\n", sendQueues_size);

    exit(0);
}

int getQueue(char * name){
    int i;

    for (i = 0; i < sendQueues_size; i++) {
        if (!strcmp(name, clientIDs[i]))
            return sendQueues[i];
    }

    return -1;
}

int main(int argc, char ** argv) {

    sendQueues = malloc(MAX_CLIENTS * sizeof(mqd_t));
    recQueues  = malloc(MAX_CLIENTS * sizeof(mqd_t));
    clientIDs  = malloc(MAX_CLIENTS * sizeof(char*));

    handShakeQueue = createQueue(HANDSHAKEQUEUE);
    printf("Handshake queue created. Server running...\n");

    //atexit(cleanUp);
    signal(SIGINT, cleanUp);

    int rec_Status;
    idMsg recId;
    char buff[128];
    message msg;

    while (1) {
        sleep(1);

        rec_Status = msgrcv(handShakeQueue, &recId, sizeof(recId.id), 0, IPC_NOWAIT);
        if (rec_Status >= 0) {
            sprintf(buff, "%s", recId.id);
            int send_id = msgget(IPC_PRIVATE, IPC_CREAT | S_IRUSR | S_IWUSR | S_IWGRP);
            int rec_id = msgget(IPC_PRIVATE, IPC_CREAT | S_IRUSR | S_IWUSR | S_IWGRP);

            printf("rec=%d send=%d\n", rec_id, send_id);

            recId.mtype = SERVER;
            sprintf(recId.id, "%d", send_id);
            msgsnd(handShakeQueue, &recId, sizeof(recId.id), 0);
            sprintf(recId.id, "%d", rec_id);
            msgsnd(handShakeQueue, &recId, sizeof(recId.id), 0);

            sendQueues[sendQueues_size] = send_id;
            recQueues[sendQueues_size]  = rec_id;
            clientIDs[sendQueues_size]  = malloc(sizeof(char) * strlen(buff));
            sprintf(clientIDs[sendQueues_size], "%s\n", buff);
            printf("Registering user %s.\n", buff);
            sendQueues_size++;
        }

        int i;
        for (i = 0; i < sendQueues_size; i++) {
            int rec_Status = msgrcv(recQueues[i], &msg, sizeof(msg.time) + sizeof(msg.to) + sizeof(msg.content), 0, IPC_NOWAIT);
            if (rec_Status >= 0) {
                int to = getQueue(msg.to);
                if (to != -1) {
                    sprintf(msg.to, "%s", clientIDs[i]);
                } else {
                    sprintf(msg.content, ">> Message could not be delivered, try later. <<<\n");
                    to = sendQueues[i];
                }

                msg.mtype = SERVER;
                rec_Status = msgsnd(to, &msg, sizeof(msg.time) + sizeof(msg.to) + sizeof(msg.content) , 0);

                if (rec_Status < 0) {
                    printf("Message could not be forwared, no such user.\n");
                    return 1;
                }
            }
        }
    }

    return 0;
}
