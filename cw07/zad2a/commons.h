#ifndef _COMMONS_H_
#define _COMMONS_H_

#define SERVERQUEUE "/servers"
#define HSQ_SIZE 10

#define CLIENT 1
#define SERVER 2

typedef struct message {
    long mtype;
    pid_t pid;
    char name[128];
    char content[256];
} message;

typedef struct text {
    long mtype;
    char text[128];
} text;

int createQueue(char * name){
	return msgget(ftok(name, 1), IPC_CREAT | S_IRUSR | S_IWUSR | S_IWGRP);
}

#endif
