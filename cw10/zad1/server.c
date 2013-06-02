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

#include "commons.h"

int unixSocket = 0;
int internetSocket = 0;
int userCounter = 0;
User * users[MAXCLIENTS];
char * _socketFile;

void printHelp() {
    printf("Arguments:\n-f <path> path to the unix socket's file\n-p <number> port used by the internet socket.\n");
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

    return sock;
}

int getUnixSocket(char* file) {
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

    return sock;
}

void sendUsers(int sock, struct sockaddr* client_name) {
	int c = 0;

	int i;
	for (i = 0; i < MAXCLIENTS; i++) {
		if (users[i]) {
			c++;
		}
	}

	UserInfoHeader uih;
	uih.count = c;

	if (sendto(sock, &uih, sizeof(UserInfoHeader), 0, client_name, sizeof(*client_name)) < 0) {
		printf("Could not send message header!\n");
        exit(1);
	}

	for (i = 0; i < MAXCLIENTS; i++) {
		if (users[i]) {
			if (sendto(sock, users[i], sizeof(User), 0, client_name, sizeof(*client_name)) < 0) {
				printf("Could not send user %d to the user.\n", i);
				exit(1);
			}
		}
	}
}

void serveRequest(int sock) {
	Request req;
	struct sockaddr client_name;

	socklen_t siz = (socklen_t) sizeof(client_name);
	if(recvfrom(sock, &req, sizeof(Request), MSG_DONTWAIT, &client_name, &siz) < 0) {
		printf("Could not receive message!");
		exit(2);
	}

	switch(req.type) {
		case REQ_LOGIN:
			users[userCounter] = (User*) malloc(sizeof(User));
			users[userCounter]->id = userCounter;
			users[userCounter]->size = req.size;
			users[userCounter]->mode = req.mode;
			strcpy(users[userCounter]->name, req.name);
			users[userCounter]->client_name = (struct sockaddr*)malloc(sizeof(struct sockaddr));
			memcpy(users[userCounter]->client_name, &client_name, sizeof(client_name));

			if(sendto(sock, &userCounter, sizeof(int), 0, &client_name, req.size) == -1) {
				printf("Could not send ID.\n");
				exit(3);
			}

			userCounter = (userCounter+1) % MAXCLIENTS;
			break;
		case REQ_GETUSERS:
			sendUsers(sock, &client_name);
			break;
		case REQ_USERINFO: {
			if(users[req.value]) {
				int s;
				s = users[req.value]->mode ? internetSocket : unixSocket;
				if(sendto(s, &req.id, sizeof(int), 0, users[req.value]->client_name, users[req.value]->size) == -1)
				{
					printf("%s\n", strerror(errno));
					printf("Nie mozna wyslac ID!\n");
					exit(1);
				}
			} else {
				struct Info info;
				if(sendto(sock, &info, sizeof(Info), 0, &client_name, users[req.id]->size) == -1)
				{
					printf("%s\n", strerror(errno));
					printf("Nie mozna wyslac ID!\n");
					exit(1);
				}
			}
			break;
		}
		case REQ_RETUSERINFO: {
			Info info;

			if(recvfrom(sock, &info, sizeof(Info), 0, &client_name, (socklen_t*)&users[req.id]->size) == -1) {
				printf("Nie mozna odebrac\n");
				exit(1);
			}

			int s;
			s = users[req.value]->mode ? internetSocket : unixSocket;
			if(users[req.value])
				if(sendto(s, &info, sizeof(Info), 0, users[req.value]->client_name, users[req.value]->size) == -1)
				{
					printf("%s\n", strerror(errno));
					printf("Nie mozna wyslac ID!\n");
					exit(1);
				}
			break;
		}
		case REQ_LOGOUT: {
			free(users[req.id]->client_name);
			free(users[req.id]);
			users[req.id] = NULL;

			break;
		}
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
    free(_socketFile);

	exit(0);
}

int main(int argc, char ** argv) {
	int port = DEFAULT_PORT;
	_socketFile = malloc(sizeof(char) * HOST_NAME_MAX);

	signal(SIGINT, serverDestroy);

	if(argc < 2) {
	    printHelp();
		exit(1);
	}

	int c;
	while((c = getopt(argc, argv, "f:p:")) != -1) {
		switch(c) {
			case 'p':
				port = atoi(optarg);
				break;
			case 'f':
				strcpy(_socketFile, optarg);
				break;
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

	serverRun();

	return 0;
}
