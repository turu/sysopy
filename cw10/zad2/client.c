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
#include <sys/un.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/sysinfo.h>

#include "commons.h"

int id;
int sock;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t requestThread;
int logged_out = 1;

void printHelp() {
    printf("Arguments:\n-m <UNIX|INTERNET> mode of operation\n-a <address> \n-f <path> \n-p <port>\n");
}

int getInternetSocket(char * address, int port) {
    struct sockaddr_in srv_name_in;
	int sock;

	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Could not create socket!\n");
        exit(1);
    }

    srv_name_in.sin_family = AF_INET;
    srv_name_in.sin_port = htons(port);
    if (inet_pton(AF_INET, address, &srv_name_in.sin_addr) < 0) {
        close(sock);
        printf("Connection failure.\n");
        exit(1);
    }

    if (connect(sock, (struct sockaddr*)&srv_name_in, sizeof(srv_name_in)) < 0) {
        printf("Connection failure.\n");
        exit(1);
    }

    printf("Internet socket created, connection established.\n");
    return sock;
}

int getUnixSocket(char * file) {
    struct sockaddr_un srv_name_un;
	int sock;

	if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) == -1) {
        printf("Could not create socket!\n");
        exit(1);
    }

    srv_name_un.sun_family = AF_UNIX;
    strcpy(srv_name_un.sun_path, file);

    if (connect(sock, (struct sockaddr*)&srv_name_un, sizeof(srv_name_un)) < 0) {
        printf("Connection failure.\n");
        exit(1);
    }

    printf("Unix socket created, connection established.\n");
    return sock;
}

void getUsers() {
	Request req;
	req.type = REQ_GETUSERS;
	req.id = id;

	if (send(sock, &req, sizeof(Request), 0) < 0) {
		printf("Could not send users request.\n");
		close(sock);
		exit(1);
	}

	if (pthread_mutex_lock(&mutex) != 0) {
        printf("Could not set lock!\n");
		exit(1);
	}

	UserInfoHeader uih;
	if (recv(sock, &uih, sizeof(UserInfoHeader), 0) < 0) {
		printf("Could not receive user info header!");
	    exit(1);
	}

	printf(">>>>>>>>>>>>>>>>>>>>>>>> USERS <<<<<<<<<<<<<<<<<<<<<<<<\n");

	User user;
	int i;
	for (i = 0; i < uih.user_count; i++) {
		if (recv(sock, &user, sizeof(User), 0) < 0) {
			printf("Could not receive info for %d'th number!\n", i);
			exit(1);
		}

		printf("User id: %d\tname: %s\n", user.id, user.name);
	}

	printf("---------------------------------------------------------\n");

	if (pthread_mutex_unlock(&mutex) != 0) {
		printf("Could not free lock!\n");
		exit(1);
	}
}

void login() {
	Request req;
	gethostname(req.name, sizeof(req.name));
	sprintf(req.name, "%s@%s", req.name,  gethostent()->h_name);
	req.id = -1;
	req.type = REQ_LOGIN;

	if (send(sock, &req, sizeof(Request), 0) < 0) {
		printf("Could not login to %s server addr: %d\n", req.name, sock);
		close(sock);
		exit(1);
	}

	if (recv(sock, &id, sizeof(int), 0) < 0) {
		printf("Could not receive id from server!\n");
		exit(1);
	}

    printf("Login successfull. Logged in as id %d\n", id);
    logged_out = 0;
	getUsers();
}

void logout() {
	Request req;
	gethostname(req.name, sizeof(req.name));
	sprintf(req.name, "%s@%s", req.name,  gethostent()->h_name);
	req.id = id;
	req.type = REQ_LOGOUT;

	if (send(sock, &req, sizeof(Request), 0) < 0) {
		printf("Failure sending logout request!\n");
		close(sock);
		exit(1);
	}
	logged_out = 1;
	printf("User logged out.\n");
}

void getRemoteCommandResult() {
    int targetId;
    printf("Type target user id: ");
    scanf("%d", &targetId);
    printf("Type command to execute: ");
    fflush(stdout);

    Request req;
    req.type = REQ_EXECUTE;
    req.value = targetId;
    req.id = id;
    gets(req.name); // awful
    gets(req.name); // hack

	if (pthread_mutex_lock(&mutex) != 0) {
		printf("Could not set lock!");
		exit(1);
	}

	if (send(sock, &req, sizeof(Request), 0) < 0) {
		printf("Could not deliver command request!\n");
		close(sock);
		exit(1);
	}

	CommandRequest cr;
	if (recv(sock, &cr, sizeof(CommandRequest), 0) < 0) {
		printf("Could not receive server response!\n");
	    exit(1);
	}

    printf(">>>>>>>>>>>>>>>>>>>>>>>>> CMD <<<<<<<<<<<<<<<<<<<<<<<<<\n");
	printf("Response received from user %d.\n", cr.targetId);
	printf("Command: %s\n", cr.command_name);
	printf("%s\n", cr.value);
	printf("---------------------------------------------------------\n");

	if(pthread_mutex_unlock(&mutex) != 0) {
		printf("Could not free lock!\n");
		exit(1);
	}
}

void clientRun() {
	int c;

	while (1) {
		printf("Type what to do:\n1 - Retrieve a list of logged users\n2 - Execute remote command\n3 - Logout & Exit\n");
		fflush(stdout);
		scanf("%i", &c);
		switch (c) {
			case 1:
				getUsers();
				break;
			case 2:
				getRemoteCommandResult();
				break;
			case 3:
				logout();
				return;
		}
	}
}

void executeRemoteCommand(CommandRequest cr) {
    Request req;
    req.type = REQ_RESP_EXECUTE;
    req.id = id;
    req.value = cr.callerId;

	if (send(sock, &req, sizeof(Request), 0) < 0) {
		printf("Could not initiate execute response\n");
		close(sock);
		exit(1);
	}

	FILE * f;

	if ((f = popen(cr.command_name, "r")) == NULL) {
	    printf("Could not execute given command: %s.\n", cr.command_name);
	    sprintf(cr.value, "Could not execute given command!");
	} else {
        fread(cr.value, sizeof(char), 255, f);
        pclose(f);
	}

	if (send(sock, &cr, sizeof(CommandRequest), 0) < 0) {
		printf("Could not transfer command response.\n");
		close(sock);
		exit(1);
	}

    printf("Remote command execution request received from user %d.\nCommand: %s\n", cr.callerId, cr.command_name);
}

void * requestListenerRun(void * args) {
    CommandRequest cr;

	while (1) {
		if (pthread_mutex_lock(&mutex) != 0) {
			printf("Could not set lock!\n");
			exit(1);
		}

		if (recv(sock, &cr, sizeof(CommandRequest), MSG_DONTWAIT) < 0) {
			if(errno != EAGAIN && errno != EWOULDBLOCK && logged_out != 1) {
				printf("Could not receive command request!\n");
				exit(1);
			}
		} else {
			executeRemoteCommand(cr);
		}

		if (pthread_mutex_unlock(&mutex) != 0) {
			printf("Could not free lock!\n");
			exit(1);
		}
	}
}

void serveRequests() {
	if (pthread_create(&requestThread, NULL, requestListenerRun, NULL) != 0) {
		printf("Could not create requestThread for serving requests.\n");
		exit(1);
	}

	if (pthread_detach(requestThread) != 0) {
		printf("Could not set request requestThread to detached state.\n");
		exit(1);
	}
}

void closeSession(int a) {
	logout();
	exit(1);
}

int main(int argc, char ** argv) {
	int port = DEFAULT_PORT;
	int mode = DEFAULT_MODE;
	char address[HOST_NAME_MAX] = DEFAULT_ADDRESS;
	char file[HOST_NAME_MAX] = DEFAULT_FILE;
	int sock;

	signal(SIGINT, closeSession);

	if (argc < 2) {
		printHelp();
		exit(1);
	}

	int c;
	while ((c = getopt(argc, argv, "m:a:p:f")) != -1) {
		switch (c) {
			case 'm':
				if(!strcmp(optarg, "UNIX")) {
					mode = MODE_UNIX;
				} else {
					mode = MODE_INET;
				}

				break;
			case 'a':
				strcpy(address, optarg);
				break;
			case 'f':
				strcpy(file, optarg);
				break;
			case 'p':
				port = atoi(optarg);
				break;
			default:
                printHelp();
				exit(0);
		}
	}

	printf("Arguments parsed! %s\n", file);

	if (mode == MODE_INET) {
        sock = getInternetSocket(address, port);
    } else {
        sock = getUnixSocket(file);
    }

	login();

	serveRequests();
	clientRun();

	close(sock);

	return 0;
}
