#ifndef _COMMONS_H_
#define _COMMONS_H_

#define HANDSHAKEQUEUE "/servers"
#define MAX_CLIENTS 32
#define HSQ_SIZE 10

#define CLIENT 1
#define SERVER 2

typedef struct message {
    long mtype;
    time_t time;
    char to[128];
    char content[256];
} message;

typedef struct idMsg {
    long mtype;
    char id[128];
} idMsg;

int createQueue(char * name){
	return msgget(ftok(name, 1), IPC_CREAT | S_IRUSR | S_IWUSR | S_IWGRP);
}

#endif
