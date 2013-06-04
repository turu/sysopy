#include <sys/resource.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
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
User * users[MAXCLIENTS];
char * _socketFile;

void printHelp() {
    printf("Arguments:\n-f <path> path to the unix socket's file\n-p <number> port used by the internet socket.\n-h print help\n");
}

int getInternetSocket(int port) {
	struct sockaddr_in srv_name;
	int sock;

	if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
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

    printf("Internet socket created.\n");
    return sock;
}

int getUnixSocket(char * file) {
	struct sockaddr_un srv_name;
	int sock;

	unlink(file);

	if ((sock = socket(PF_UNIX, SOCK_DGRAM, 0)) == -1) {
        printf("Could not create socket!\n");
        exit(1);
    }

    srv_name.sun_family = AF_UNIX;
    strcpy(srv_name.sun_path, file);

    if(bind(sock, (struct sockaddr*)&srv_name, SUN_LEN(&srv_name)) < 0) {
    	printf("Connection failure.\n");
        exit(1);
    }

    printf("Unix socket created.\n");
    return sock;
}

void sendUsers(int sock, struct sockaddr * client_name) {
	int c = 0;

	int i;
	for (i = 0; i < MAXCLIENTS; i++) {
		if (users[i]) {
			c++;
		}
	}

	UserInfoHeader uih;
	uih.user_count = c;

	if (sendto(sock, &uih, sizeof(UserInfoHeader), 0, client_name, sizeof(*client_name)) < 0) {
		printf("Could not send message header!\n");
	}

	for (i = 0; i < MAXCLIENTS; i++) {
		if (users[i]) {
			if (sendto(sock, users[i], sizeof(User), 0, client_name, sizeof(*client_name)) < 0) {
				printf("Could not send user %d to the user.\n", i);
			}
		}
	}
}

void serveRequest(int sock) {
	Request req;
	CommandRequest cr;
	int sockId;
	struct sockaddr client_name;

	socklen_t scklen = (socklen_t) sizeof(client_name);
	if(recvfrom(sock, &req, sizeof(Request), MSG_DONTWAIT, &client_name, &scklen) < 0) {
		//printf("Could not receive message!\n"); <- no pending messages for the server
		//printf(".");
		return;
	}

	switch (req.type) {
		case REQ_LOGIN:
            printf("Received login request.\n");
			users[userCounter] = (User*) malloc(sizeof(User));
			users[userCounter]->id = userCounter;
			users[userCounter]->size = req.size;
			users[userCounter]->mode = req.mode;
			strcpy(users[userCounter]->name, req.name);
			users[userCounter]->client_name = (struct sockaddr*)malloc(sizeof(struct sockaddr));
			memcpy(users[userCounter]->client_name, &client_name, sizeof(client_name));

			if(sendto(sock, &userCounter, sizeof(int), 0, &client_name, req.size) == -1) {
				printf("Could not send ID %d.\n", userCounter);
			}

			userCounter = (userCounter+1) % MAXCLIENTS;
			break;
		case REQ_LOGOUT:
            printf("Received logout request from user %d.\n", req.id);
            if (users[req.id] == NULL) {
                printf("User not connected");
            }
			free(users[req.id]->client_name);
			free(users[req.id]);
			users[req.id] = NULL;
			break;
		case REQ_GETUSERS:
            printf("Received user list request.\n");
			sendUsers(sock, &client_name);
			break;
        case REQ_EXECUTE:
            printf("Received remote command request from user %d, target %d.\n", req.id, req.value);
            if (users[req.value]->mode == MODE_UNIX) {
                sockId = unixSocket;
            } else {
                sockId = internetSocket;
            }
            strcpy(cr.command_name, req.name);
            cr.targetId = req.value;
            cr.callerId = req.id;

            if (users[req.value]) {
                if(sendto(sockId, &cr, sizeof(CommandRequest), 0, users[req.value]->client_name, users[req.value]->size) < 0) {
					printf("Could not send command execution request!\n");
				}
            } else {
                strcpy(cr.value, "Target user unavailable, try again later.\n");
                if(sendto(sockId, &cr, sizeof(CommandRequest), 0, &client_name, users[req.id]->size) < 0) {
					printf("Could not send command execution failure response!\n");
				}
            }
            break;
        case REQ_RESP_EXECUTE:
            if(recvfrom(sock, &cr, sizeof(CommandRequest), 0, &client_name, (socklen_t*)&users[req.id]->size) < 0) {
				printf("Could not receive data.\n");
			}
			printf("Received remote command request response. Caller id %d, target id %d.\n", cr.callerId, cr.targetId);

            if (users[req.value]->mode == MODE_UNIX) {
                sockId = unixSocket;
            } else {
                sockId = internetSocket;
            }

            if (users[req.value]) {
				if(sendto(sockId, &cr, sizeof(CommandRequest), 0, users[req.value]->client_name, users[req.value]->size) < 0) {
					printf("Could not send command execution response to user %d!\n", req.value);
				}
            }

            break;
	}
}

void serverRun() {
	while (1) {
		serveRequest(unixSocket);
		serveRequest(internetSocket);
	}
}

void serverDestroy(int arg) {
	int i;
	close(unixSocket);
	close(internetSocket);
	for (i = 0; i < MAXCLIENTS; ++i) {
		if (users[i] != NULL) {
			free(users[i]->client_name);
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
	printf("Server initialized.\n");

	serverRun();

	return 0;
}
