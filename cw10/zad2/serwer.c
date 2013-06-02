#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/resource.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <getopt.h>
#include <netdb.h>
#include <pthread.h>

#include "structures.h"

int socketUNIX = 0, socketINTERNET = 0;
struct LoggedUser* lu[LOGGEDMAX];
int counter = 0;
pthread_t threads[LOGGEDMAX];
int t_counter = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_c = PTHREAD_MUTEX_INITIALIZER;

int size_c;

void argsParsing(int argc, char* argv[], int* port, char* file) {
	if(argc < 2) {
		printf("serwer -p [port] -f [file]\n");
		exit(0);
	}

	const char* const short_opt = "p:f:";
	const struct option long_opt[] =
		{{"port", 1, NULL, 'p'},
		{"file", 1, NULL, 'f'},
		{NULL, 0, NULL, 0}};

	int next_opt;
	while((next_opt = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1) {
		switch(next_opt) {
			case 'p': {
				*port = atoi(optarg);
				break;
			}
			case 'f': {
				file = optarg;
				break;
			}
			default: {
				printf("serwer -p [port] -f [file]\n");
				exit(0);
			}
		}
	}
}

int createSocketINTERNET(int port) {
	struct sockaddr_in srv_name;
	int sock;

	if((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
       		printf("Nie mozna otworzyc soketu\n");
       		exit(1);
    	}

    	srv_name.sin_family = AF_INET;
    	srv_name.sin_port = htons(port);
	srv_name.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sock, (struct sockaddr*)&srv_name, sizeof(srv_name)) == -1) {
    		printf("Nie mozna polaczyc\n");
        	exit(1);
    	}

    	if(listen(sock, SOMAXCONN) == -1) {
    		printf("Nie mozna wykonac 'listen'\n");
    		close(sock);
    		exit(1);
    	}

    return sock;
}


int createSocketUNIX(char* file) {
	struct sockaddr_un srv_name;
	int sock;

	unlink(file);

	if((sock = socket(PF_UNIX, SOCK_STREAM, 0)) == -1) {
        	printf("Nie mozna otworzyc soketu\n");
        	exit(1);
    	}

    	srv_name.sun_family = AF_UNIX;
    	strcpy(srv_name.sun_path, file);

    	if(bind(sock, (struct sockaddr*)&srv_name, SUN_LEN(&srv_name)) == -1) {
    		printf("Nie mozna polaczyc\n");
        	exit(1);
    	}

    	if(listen(sock, SOMAXCONN) == -1) {
    		printf("Nie mozna wykonac 'listen'\n");
    		close(sock);
    		exit(1);
    	}

    return sock;
}

void createSockets(int port, char* file) {
	socketUNIX = createSocketUNIX(file);
	socketINTERNET = createSocketINTERNET(port);
}

void sendLoggedUsers(int sock) {
	int err;
	if((err = pthread_mutex_lock(&mutex)) != 0) {
		printf("Nie mozna zablokowac mutexu\n");
		exit(1);
	}

	int c = 0;

	int i;
	for(i = 0; i < LOGGEDMAX; ++i)
		if(lu[i])
			++c;

	struct PendingMessages pm;
	pm.count = c;

	if(send(sock, &pm, sizeof(struct PendingMessages), 0) == -1) {
		printf("Nie mozna wyslac wiadomosci!\n");
	    	exit(1);
	}

	for(i = 0; i < LOGGEDMAX; ++i) {
		if(lu[i]) {
			if(send(sock, lu[i], sizeof(struct LoggedUser), 0) == -1) {
				printf("Nie mozna wyslac wiadomosci!\n");
				exit(1);
			}
		}
	}


	if((err = pthread_mutex_unlock(&mutex)) != 0) {
		printf("Nie mozna odblokowac mutexu\n");
		exit(1);
	}
}

void getRequest(int sock) {
	for(;;) {
		struct Request req;
		if(recv(sock, &req, sizeof(struct Request), 0) == -1) {
			if(errno != EAGAIN && errno != EWOULDBLOCK) {
				printf("Nie mozna odebrac wiadomosci!\n");
				exit(1);
			}
			else
				return;
		}

		switch(req.type) {
			case REQ_LOGIN: {
				int err;
				if((err = pthread_mutex_lock(&mutex)) != 0) {
					printf("Nie mozna zablokowac mutexu\n");
					exit(1);
				}

				lu[counter] = (struct LoggedUser*)malloc(sizeof(struct LoggedUser));
				lu[counter]->id = counter;
				strcpy(lu[counter]->name, req.name);
				lu[counter]->socket = sock;

				if(send(sock, &counter, sizeof(int), 0) == -1) {
					printf("Nie mozna wyslac ID!\n");
					exit(1);
				}

				counter = (counter+1)%LOGGEDMAX;

				if((err = pthread_mutex_unlock(&mutex)) != 0) {
					printf("Nie mozna odblokowac mutexu\n");
					exit(1);
				}
				break;
			}
			case REQ_LOGGEDUSERS: {
				sendLoggedUsers(sock);
				break;
			}
			case REQ_USERINFO: {
				if(lu[req.value]) {
					if(send(lu[req.value]->socket, &req.id, sizeof(int), 0) == -1) {
						printf("Nie mozna wyslac ID!\n");
						exit(1);
					}
				}
				else {
					struct Info info;
					if(send(sock, &info, sizeof(struct Info), 0) == -1) {
						printf("Nie mozna wyslac ID!\n");
						exit(1);
					}
				}
				break;
			}
			case REQ_RETUSERINFO: {
				struct Info info;
				if(recv(sock, &info, sizeof(struct Info), 0) == -1) {
					printf("Nie mozna odebrac\n");
					exit(1);
				}

				if(lu[req.value])
					if(send(lu[req.value]->socket, &info, sizeof(struct Info), 0) == -1) {
						printf("Nie mozna wyslac ID!\n");
						exit(1);
					}
				break;
			}
			case REQ_LOGOUT: {
				free(lu[req.id]);
				lu[req.id] = NULL;

				pthread_exit(0);

				break;
			}
		}
	}
}

void* threadWork(void* arg) {
	int sock = *(int*)arg;

	getRequest(sock);

	pthread_exit(NULL);
}

void* listenForMessages(void* arg) {
	int listensocket = *(int*)arg;
	int sock;
	struct sockaddr cli_name;
	size_t size = sizeof(cli_name);


	for(;;){
		if((sock = accept(listensocket, &cli_name, &size)) == -1) {
			printf("Nie mozna zaakceptowac\n");
		    	pthread_exit(NULL);
		}

		int err;
		if((err = pthread_mutex_lock(&mutex_c)) != 0) {
			printf("Nie mozna zablokowac mutexu\n");
			pthread_exit(NULL);
		}

		if((err = pthread_create(&threads[t_counter], NULL, &threadWork, &sock)) != 0) {
			printf("%s\n", strerror(err));
			pthread_exit(NULL);
		}
		if((err = pthread_detach(threads[t_counter])) != 0) {
			printf("%s\n", strerror(err));
			pthread_exit(NULL);
		}

		t_counter = (t_counter+1)%LOGGEDMAX;

		if((err = pthread_mutex_unlock(&mutex_c)) != 0) {
			printf("Nie mozna odblokowac mutexu\n");
			pthread_exit(NULL);
		}
	}
}

void closeSession(int a) {
	int i;
	close(socketUNIX);
	close(socketINTERNET);
	for(i = 0; i < LOGGEDMAX; ++i) {
		if(lu[i])
			free(lu[i]);
	}
	exit(1);
}

int main(int argc, char* argv[])
{
	int port = DEFAULT_PORT;
	char file[HOST_NAME_MAX] = DEFAULT_FILE;

	signal(SIGINT, closeSession);

	argsParsing(argc, argv, &port, file);

	int i;
	for(i = 0; i < LOGGEDMAX; ++i)
		lu[i] = NULL;

	createSockets(port, file);


	pthread_t tUNIX;
	pthread_t tINTERNET;

	int err;
	if((err = pthread_create(&tUNIX, NULL, &listenForMessages, &socketUNIX)) != 0) {
		printf("%s\n", strerror(err));
		return 1;
	}
	if((err = pthread_detach(tUNIX)) != 0) {
		printf("%s\n", strerror(err));
		return 1;
	}

	if((err = pthread_create(&tINTERNET, NULL, &listenForMessages, &socketINTERNET)) != 0) {
		printf("%s\n", strerror(err));
		return 1;
	}
	if((err = pthread_detach(tINTERNET)) != 0)
	{
		printf("%s\n", strerror(err));
		return 1;
	}

	for(;;){}

	return 0;
}
