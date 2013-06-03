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

struct sockaddr * srv_name;
struct sockaddr_in srv_name_in;
struct sockaddr_un srv_name_un;
struct sockaddr_un cli_name_un;
int id;
Listener listener;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t requestThread;
int logged_out = 1;

void printHelp() {
    printf("Arguments:\n-m <UNIX|INTERNET> mode of operation\n-a <address> \n-f <path> \n-p <port>\n");
}

int getInternetSocket(char * address, int port) {
	int sock;

	if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
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

    srv_name = (struct sockaddr*)&srv_name_in;

    printf("Internet socket created.\n");
    return sock;
}

int getUnixSocket(char * file) {
    char path[256];
	int sock;
	int pid = getpid();
	sprintf(path, "/tmp/c%i", pid);

	unlink(path);

	if ((sock = socket(PF_UNIX, SOCK_DGRAM, 0)) == -1) {
        printf("Could not create socket!\n");
        exit(1);
    }

    cli_name_un.sun_family = AF_UNIX;
    strncpy(cli_name_un.sun_path, path, strlen(path));

    if(bind(sock, (struct sockaddr*)&cli_name_un, SUN_LEN(&cli_name_un)) < 0) {
    	printf("Connection failure.\n");
        exit(1);
    }

    srv_name_un.sun_family = AF_UNIX;
    strcpy(srv_name_un.sun_path, file);
    srv_name = (struct sockaddr*)&srv_name_un;

    printf("Unix socket created.\n");
    return sock;
}

void getUsers(int sock, int mode) {
	Request req;
	req.type = REQ_GETUSERS;
	req.id = id;

	socklen_t size;
	if (mode) {
		size = (socklen_t) sizeof(srv_name_in);
	} else {
		size = (socklen_t) SUN_LEN(&srv_name_un);
	}

	if (sendto(sock, &req, sizeof(Request), 0, srv_name, size) < 0) {
		printf("Could not send users request.\n");
		close(sock);
		exit(1);
	}

	if (pthread_mutex_lock(&mutex) != 0) {
        printf("Could not set lock!\n");
		exit(1);
	}

	UserInfoHeader uih;
	if (recvfrom(sock, &uih, sizeof(UserInfoHeader), 0, srv_name, &size) < 0) {
		printf("Could not receive user info header!");
	    exit(1);
	}

	printf(">>>>>>>>>>>>>>>>>>>>>>>> USERS <<<<<<<<<<<<<<<<<<<<<<<<\n");

	User user;
	int i;
	for (i = 0; i < uih.user_count; i++) {
		if (recvfrom(sock, &user, sizeof(User), 0, srv_name, &size) < 0) {
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

void login(int sock, int mode) {
	Request req;
	gethostname(req.name, sizeof(req.name));
	sprintf(req.name, "%s@%s", req.name,  gethostent()->h_name);
	req.id = -1;
	req.type = REQ_LOGIN;
	req.mode = mode;

	if (mode) {
		req.size = sizeof(cli_name_un);
	} else {
		req.size = SUN_LEN(&cli_name_un);
	}

	socklen_t size;
	if (mode) {
		size = (socklen_t) sizeof(srv_name_in);
	} else {
		size = (socklen_t) SUN_LEN(&srv_name_un);
	}

	if (sendto(sock, &req, sizeof(Request), 0, srv_name, size) < 0) {
		printf("Could not login to %s server addr: %u\n", req.name, (unsigned int)srv_name_in.sin_addr.s_addr);
		close(sock);
		exit(1);
	}

	if (recvfrom(sock, &id, sizeof(int), 0, srv_name, &size) < 0) {
		printf("Could not receive id from server!\n");
		exit(1);
	}

    printf("Login successfull. Logged in as id %d\n", id);
    logged_out = 0;
	getUsers(sock, mode);
}

void logout(int sock, int mode) {
	Request req;
	gethostname(req.name, sizeof(req.name));
	sprintf(req.name, "%s : %s", req.name,  gethostent()->h_name);
	req.id = id;
	req.type = REQ_LOGOUT;

	socklen_t size;
	if (mode) {
		size = (socklen_t) sizeof(srv_name_in);
	} else {
		size = (socklen_t) SUN_LEN(&srv_name_un);
	}

	if (sendto(sock, &req, sizeof(Request), 0, srv_name, size) < 0) {
		printf("Failure sending logout request!\n");
		close(sock);
		exit(1);
	}
	logged_out = 1;
	printf("User logged out.\n");
}

void getRemoteCommandResult(int sock, int mode) {
    int targetId;
    printf("Type target user id: ");
    scanf("%d", &targetId);
    printf("Type command to execute: ");
    fflush(stdout);

    Request req;
    req.type = REQ_EXECUTE;
    req.value = targetId;
    req.id = id;
    gets(req.name);
    gets(req.name);

    socklen_t size;
	if (mode) {
		size = (socklen_t) sizeof(srv_name_in);
	} else {
		size = (socklen_t) SUN_LEN(&srv_name_un);
	}

	if (pthread_mutex_lock(&mutex) != 0) {
		printf("Could not set lock!");
		exit(1);
	}

	if (sendto(sock, &req, sizeof(Request), 0, srv_name, size) < 0) {
		printf("Could not deliver command request!\n");
		close(sock);
		exit(1);
	}

	CommandRequest cr;
	if (recvfrom(sock, &cr, sizeof(CommandRequest), 0, srv_name, &size) < 0) {
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

void clientRun(int sock, int mode) {
	int c;

	while (1) {
		printf("Type what to do:\n1 - Retrieve a list of logged users\n2 - Execute remote command\n3 - Logout & Exit\n");
		fflush(stdout);
		scanf("%i", &c);
		switch (c) {
			case 1:
				getUsers(sock, mode);
				break;
			case 2:
				getRemoteCommandResult(sock, mode);
				break;
			case 3:
				logout(sock, mode);
				return;
		}
	}
}

void executeRemoteCommand(CommandRequest cr) {
    Request req;
    req.type = REQ_RESP_EXECUTE;
    req.id = id;
    req.value = cr.callerId;

    socklen_t size;
	if (listener.mode) {
		size = (socklen_t) sizeof(srv_name_in);
	} else {
		size = (socklen_t) SUN_LEN(&srv_name_un);
	}

	if (sendto(listener.socket, &req, sizeof(Request), 0, srv_name, size) < 0) {
		printf("Could not initiate execute response\n");
		close(listener.socket);
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

	if (sendto(listener.socket, &cr, sizeof(CommandRequest), 0, srv_name, size) < 0) {
		printf("Could not transfer command response.\n");
		close(listener.socket);
		exit(1);
	}

    printf("Remote command execution request received from user %d.\nCommand: %s\n", cr.callerId, cr.command_name);
}

void * requestListenerRun(void * args) {
	socklen_t size;
	if (listener.mode) {
		size = (socklen_t) sizeof(srv_name_in);
	} else {
 		size = (socklen_t) SUN_LEN(&srv_name_un);
	}

    CommandRequest cr;

	while (1) {
		if (pthread_mutex_lock(&mutex) != 0) {
			printf("Could not set lock!\n");
			exit(1);
		}

		if (recvfrom(listener.socket, &cr, sizeof(CommandRequest), MSG_DONTWAIT, srv_name, &size) < 0) {
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

void serveRequests(sock, mode) {
	listener.mode = mode;
	listener.socket = sock;

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
	logout(listener.socket, listener.mode);
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

	login(sock, mode);

	serveRequests(sock, mode);
	clientRun(sock, mode);

	close(sock);

	return 0;
}
