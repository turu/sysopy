#include <sys/resource.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <sys/un.h>
#include <getopt.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>

#include "commons.h"

int unixSocket = 0;
int internetSocket = 0;
int userCounter = 0;
int clientThreadCounter = 0;
User * users[MAXCLIENTS];
char * _socketFile;

pthread_t threads[MAXCLIENTS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t handShakeMutex = PTHREAD_MUTEX_INITIALIZER;

void printHelp() {
    printf("Arguments:\n-f <path> path to the unix socket's file\n-p <number> port used by the internet socket.\n-h print help\n");
}

int getInternetSocket(int port) {
	struct sockaddr_in srv_name;
	int sock;

	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Could not create socket!\n");
        exit(1);
    }

    srv_name.sin_family = AF_INET;
    srv_name.sin_port = htons(port);
    srv_name.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr*)&srv_name, sizeof(srv_name)) < 0) {
        printf("Connection failure.\n");
        exit(1);
    }

    if (listen(sock, SOMAXCONN) < 0) {
        printf("Connection failure\n");
        close(sock);
        exit(1);
    }

    printf("Internet socket created.\n");
    return sock;
}

int getUnixSocket(char * file) {
	struct sockaddr_un srv_name;
	int sock;

	unlink(file);

	if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) == -1) {
        printf("Could not create socket!\n");
        exit(1);
    }

    srv_name.sun_family = AF_UNIX;
    strcpy(srv_name.sun_path, file);

    if(bind(sock, (struct sockaddr*)&srv_name, SUN_LEN(&srv_name)) < 0) {
    	printf("Connection failure.\n");
        exit(1);
    }

    if (listen(sock, SOMAXCONN) < 0) {
        printf("Connection failure\n");
        close(sock);
        exit(1);
    }

    printf("Unix socket created.\n");
    return sock;
}

void sendUsers(int sock) {
    if (pthread_mutex_lock(&mutex) != 0) {
        printf("Could not set lock!\n");
        exit(1);
    }
	int c = 0;

	int i;
	for (i = 0; i < MAXCLIENTS; i++) {
		if (users[i]) {
			c++;
		}
	}

	UserInfoHeader uih;
	uih.user_count = c;

	if (send(sock, &uih, sizeof(UserInfoHeader), 0) < 0) {
		printf("Could not send message header!\n");
	}

	for (i = 0; i < MAXCLIENTS; i++) {
		if (users[i]) {
			if (send(sock, users[i], sizeof(User), 0) < 0) {
				printf("Could not send user %d to the user.\n", i);
			}
		}
	}

	if (pthread_mutex_unlock(&mutex) != 0) {
        printf("Could not free lock!\n");
        exit(1);
    }
}

void serveRequest(int sock) {
	Request req;
	CommandRequest cr;

	if(recv(sock, &req, sizeof(Request), 0) < 0) {
		//printf("Could not receive message!\n"); <- no pending messages for the server
		//printf(".");
		return;
	}

	switch (req.type) {
		case REQ_LOGIN:
            if (pthread_mutex_lock(&mutex) != 0) {
                printf("Could not set lock!\n");
                exit(1);
            }

            printf("Received login request.\n");
			users[userCounter] = (User*) malloc(sizeof(User));
			users[userCounter]->id = userCounter;
			users[userCounter]->sock = sock;
			strcpy(users[userCounter]->name, req.name);

			if(send(sock, &userCounter, sizeof(int), 0) == -1) {
				printf("Could not send ID %d.\n", userCounter);
			}

			userCounter = (userCounter + 1) % MAXCLIENTS;

			if (pthread_mutex_unlock(&mutex) != 0) {
                printf("Could not free lock!\n");
                exit(1);
            }
			break;
		case REQ_LOGOUT:
            printf("Received logout request from user %d.\n", req.id);
            if (users[req.id] == NULL) {
                printf("User not connected");
            }
			free(users[req.id]);
			users[req.id] = NULL;
			pthread_exit(NULL);
			break;
		case REQ_GETUSERS:
            printf("Received user list request.\n");
			sendUsers(sock);
			break;
        case REQ_EXECUTE:
            printf("Received remote command request from user %d, target %d.\n", req.id, req.value);
            strcpy(cr.command_name, req.name);
            cr.targetId = req.value;
            cr.callerId = req.id;

            if (users[req.value]) {
                if(send(users[req.value]->sock, &cr, sizeof(CommandRequest), 0) < 0) {
					printf("Could not send command execution request!\n");
				}
            } else {
                strcpy(cr.value, "Target user unavailable, try again later.\n");
                if(send(sock, &cr, sizeof(CommandRequest), 0) < 0) {
					printf("Could not send command execution failure response!\n");
				}
            }
            break;
        case REQ_RESP_EXECUTE:
            if(recv(sock, &cr, sizeof(CommandRequest), 0) < 0) {
				printf("Could not receive data.\n");
			}
			printf("Received remote command request response. Caller id %d, target id %d.\n", cr.callerId, cr.targetId);

            if (users[req.value]) {
				if(send(users[req.value]->sock, &cr, sizeof(CommandRequest), 0) < 0) {
					printf("Could not send command execution response to user %d!\n", req.value);
				}
            }

            break;
	}
}

void * clientThreadRun(void * args) {
    int sock = *((int*)args);
    while (1) {
        serveRequest(sock);
    }
    pthread_exit(NULL);
}

void * serverRun(void * args) {
    int handShakeSocket = *((int*)args);
    int sock;
	struct sockaddr cli_name;
	size_t size = sizeof(cli_name);

	while (1) {
		if ((sock = accept(handShakeSocket, &cli_name, (socklen_t*)&size)) < 0) {
            printf("Could not accept new client connection!\n");
            pthread_exit(NULL);
		}

		if (pthread_mutex_lock(&handShakeMutex) != 0) {
			printf("Could not set lock.\n");
			pthread_exit(NULL);
		}

		if (pthread_create(&threads[clientThreadCounter], NULL, &clientThreadRun, &sock) != 0) {
			printf("Could not create client thread!\n");
			pthread_exit(NULL);
		}
		if (pthread_detach(threads[clientThreadCounter]) != 0) {
			printf("Could not set client thread into detached state.\n");
			pthread_exit(NULL);
		}

		clientThreadCounter = (clientThreadCounter + 1) % MAXCLIENTS;

		if (pthread_mutex_unlock(&handShakeMutex) != 0) {
			printf("Could not free lock!\n");
			pthread_exit(NULL);
		}
	}
}

void serverDestroy(int arg) {
	int i;
	close(unixSocket);
	close(internetSocket);
	for (i = 0; i < MAXCLIENTS; ++i) {
		if (users[i] != NULL) {
			free(users[i]);
		}
	}
	unlink(_socketFile);
    free(_socketFile);

    printf("Server destroyed.\n");

	exit(0);
}

int main(int argc, char ** argv) {
	int port = DEFAULT_PORT;
	_socketFile = malloc(sizeof(char) * HOST_NAME_MAX);
	_socketFile = strcpy(_socketFile, DEFAULT_FILE);

	signal(SIGINT, serverDestroy);

	int c;
	while((c = getopt(argc, argv, "f:p:h")) != -1) {
		switch(c) {
			case 'p':
				port = atoi(optarg);
				break;
			case 'f':
				strcpy(_socketFile, optarg);
				break;
            case 'h':
                printHelp();
                exit(0);
			default:
				printHelp();
				exit(0);
		}
	}

	int i;
	for (i = 0; i < MAXCLIENTS; i++) {
		users[i] = NULL;
	}

	unixSocket = getUnixSocket(_socketFile);
	internetSocket = getInternetSocket(port);

	pthread_t unixDispatchThread;

	if (pthread_create(&unixDispatchThread, NULL, &serverRun, &unixSocket) != 0) {
		printf("Could not UNIX dispatch thread!\n");
		exit(1);
	}

	if (pthread_detach(unixDispatchThread) != 0) {
		printf("Could not set unixDispatchThread into detached state!\n");
		exit(1);
	}

	printf("Server initialized.\n");
	serverRun(&internetSocket);

	return 0;
}
