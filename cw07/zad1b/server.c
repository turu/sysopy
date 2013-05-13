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

mqd_t handShakeQueue;
mqd_t * sendQueues;
mqd_t * recQueues;
char ** clientIDs;
int sendQueues_size = 0;

void closeQueue(char * name, mqd_t queue) {
    mq_close(queue);
	mq_unlink(name);
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

mqd_t getQueue(char * name){
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

    handShakeQueue = createQueue(HANDSHAKEQUEUE, sizeof(char) * 128);
    if (handShakeQueue < 0) {
        printf("Failed creating server queue.\n");
        return 2;
    }
    printf("Handshake queue (id: %d) created. Server running...\n", handShakeQueue);

    //atexit(cleanUp);
    signal(SIGINT, cleanUp);

    int rec_Status;
    char buff[128];
    char newbuff[128];
    message msg;

    while (1) {
        sleep(1);

        rec_Status = mq_receive(handShakeQueue, (char*)buff, sizeof(buff), NULL);
        if (rec_Status != -1) {
            sprintf(newbuff, "/%s", buff);
            mqd_t send_id = createQueue(newbuff, sizeof(msg));
            sprintf(newbuff,"/%sr",buff);
            mqd_t rec_id = createQueue(newbuff, sizeof(msg));

            sendQueues[sendQueues_size] = send_id;
            recQueues[sendQueues_size]  = rec_id;
            clientIDs[sendQueues_size]  = malloc(sizeof(char) * strlen(buff));
            sprintf(clientIDs[sendQueues_size], "%s\n", buff);
            printf("Registering user %s.\n", buff);
            sendQueues_size++;
        }

        int i;
        for (i = 0; i < sendQueues_size; i++) {
            int rec_Status = mq_receive(recQueues[i], (char*)(&msg), sizeof(msg), NULL);
            if (rec_Status >= 0) {
                int to = getQueue(msg.to);
                if (to != -1) {
                    sprintf(msg.to, "%s", clientIDs[i]);
                } else {
                    sprintf(msg.content, ">> Message could not be delivered, try later. <<<\n");
                    to = sendQueues[i];
                }

                rec_Status = mq_send(to, (char*)(&msg), sizeof(msg) , 0);

                if (rec_Status < 0) {
                    perror(NULL);
                    printf("Message could not be forwared, no such user.\n");
                    return 1;
                }
            }
        }
    }

    return 0;
}
