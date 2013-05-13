#ifndef _COMMONS_H_
#define _COMMONS_H_

#define SERVERQUEUE "/servers"
#define MAX_CLIENTS 32
#define HSQ_SIZE 10

typedef struct message {
    pid_t pid;
    char name[128];
    char content[256];
} message;

mqd_t createQueue(char * name, size_t size){
    struct mq_attr attr;

    attr.mq_maxmsg = HSQ_SIZE;
    attr.mq_msgsize = size;
    attr.mq_flags = 0;
    mqd_t queue = mq_open (name, O_RDWR | O_CREAT | O_NONBLOCK, 0664, &attr);
    if(queue < 0)
        perror(NULL);

	return queue;
}

#endif
