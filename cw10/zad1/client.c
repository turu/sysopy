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

struct sockaddr* 	srv_name;
struct sockaddr_in 	srv_name_in;
struct sockaddr_un 	srv_name_un;
struct sockaddr_un 	cli_name_un;
int id;
Listener listener;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t thread;

void argsParsing(int argc, char* argv[], int* mode, char* address, int* port, char* file) {
	if(argc < 2) {
		printf("klient -m [{UNIX|INTERNET}] -a [address] -f [file] -p [port]\n");
		exit(0);
	}

	const char* const short_opt = "m:a:p:f:";
	const struct option long_opt[] =
		{{"mode", 1, NULL, 'm'},
		{"address", 1, NULL, 'a'},
		{"port", 1, NULL, 'p'},
		{"file", 1, NULL, 'f'},
		{NULL, 0, NULL, 0}};

	int next_opt;
	while((next_opt = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1) {
		switch(next_opt) {
			case 'm': {
				if(!strcmp(optarg, "UNIX"))
					*mode = 0;
				else
					*mode = 1;
				break;
			}
			case 'a': {
				strcpy(address, optarg);
				break;
			}
			case 'f': {
				strcpy(file, optarg);
				break;
			}
			case 'p': {
				*port = atoi(optarg);
				break;
			}
			default: {
				printf("klient -m [{UNIX|INTERNET}] -a [address] -f [file] -p [port]\n");
				exit(0);
			}
		}
	}
}

int createSocketINTERNET(char* address, int port) {
	int csock;

	if((csock = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
        	printf("Nie mozna otworzyc soketu!\n");
       		exit(1);
    	}

    	srv_name_in.sin_family = AF_INET;
    	srv_name_in.sin_port = htons(port);
	int err = inet_pton(AF_INET, address, &srv_name_in.sin_addr);
	if(err < 0) {
     		printf("Zly adres rodziny soketow!");
      		close(csock);
      		exit(1);
    	} else if(err == 0) {
      		printf("Zly drugi parametr!");
      		close(csock);
     		exit(1);
   	}

    	srv_name = (struct sockaddr*)&srv_name_in;

    	return csock;
}


int createSocketUNIX(char* file) {
	int csock;
	char path[1024];
	int pid = getpid();
	sprintf(path, "/tmp/c%i", pid);

	unlink(path);

	if((csock = socket(PF_UNIX, SOCK_DGRAM, 0)) == -1) {
        	printf("Nie mozna otworzyc soketu\n");
        	exit(1);
    	}

    	cli_name_un.sun_family = AF_UNIX;
    	strncpy(cli_name_un.sun_path, path, strlen(path));

    	if(bind(csock, (struct sockaddr*)&cli_name_un, SUN_LEN(&cli_name_un)) == -1) {
    		printf("Nie mozna polaczyc\n");
    		printf("%s\n", strerror(errno));
        	exit(1);
    	}

    	srv_name_un.sun_family = AF_UNIX;
    	strcpy(srv_name_un.sun_path, file);

    	srv_name = (struct sockaddr*)&srv_name_un;

    	return csock;
}

int createSocket(int mode, char address[], int port, char file[]) {
	return mode ? createSocketINTERNET(address, port) : createSocketUNIX(file);
}

void getLoggedUsers(int sock, int mode) {
	Request req;
	req.type = REQ_GETUSERS;
	req.id = id;

	socklen_t size;
	if(mode)
		size = (socklen_t) sizeof(srv_name_in);
	else
		size = (socklen_t) SUN_LEN(&srv_name_un);
	if(sendto(sock, &req, sizeof(Request), 0, srv_name, size) == -1) {
		printf("Nie mozna wyslac.\n");
		printf("%s\n", strerror(errno));
		close(sock);
		exit(1);
	}

	int err;
	if((err = pthread_mutex_lock(&mutex)) != 0) {
		printf("Nie mozna zablokowac\n");
		exit(1);
	}

	UserInfoHeader uih;
	if(recvfrom(sock, &uih, sizeof(UserInfoHeader), 0, srv_name, &size) == -1) {
		printf("Nie mozna czytac wiadomosci\n");
	    exit(1);
	}

	printf("========== UZYTWKONICY ==========\n\n");

	User user;
	int i;
	for(i = 0; i < uih.count; ++i) {
		if(recvfrom(sock, &user, sizeof(User), 0, srv_name, &size) == -1) {
			printf("Nie mozna czytac wiadomosci\n");
			exit(1);
		}

		printf("ID: %i   %s\n", user.id, user.name);
	}

	printf("\n");

	if((err = pthread_mutex_unlock(&mutex)) != 0) {
		printf("Nie mozna zablokowac mutexu\n");
		exit(1);
	}
}

void login(int sock, int mode) {
	Request lg;
	gethostname(lg.name, sizeof(lg.name));
	sprintf(lg.name, "%s : %s", lg.name,  gethostent()->h_name);
	lg.id = -1;
	lg.type = REQ_LOGIN;
	lg.mode = mode;
	if(mode)
		lg.size = sizeof(cli_name_un);
	else
		lg.size = SUN_LEN(&cli_name_un);

	socklen_t size;
	if(mode)
		size = (socklen_t) sizeof(srv_name_in);
	else
		size = (socklen_t) SUN_LEN(&srv_name_un);

	if(sendto(sock, &lg, sizeof(Request), 0, srv_name, size) == -1) {
		printf("Nie mozna zalogowac, %s, serwer %u\n", lg.name, (unsigned int)srv_name_in.sin_addr.s_addr);
		printf("%s\n", strerror(errno));
		close(sock);
		exit(1);
	}

	if(recvfrom(sock, &id, sizeof(int), 0, srv_name, &size) == -1) {
		printf("Nie mozna odczytac id\n");
		exit(1);
	}

	getLoggedUsers(sock, mode);
}

void logout(int sock, int mode) {
	Request lg;
	gethostname(lg.name, sizeof(lg.name));
	sprintf(lg.name, "%s : %s", lg.name,  gethostent()->h_name);
	lg.id = id;
	lg.type = REQ_LOGOUT;

	size_t size;
	if(mode)
		size = sizeof(srv_name_in);
	else
		size = SUN_LEN(&srv_name_un);
	if(sendto(sock, &lg, sizeof(Request), 0, srv_name, size) == -1) {
		printf("%s\n", strerror(errno));
		close(sock);
		exit(1);
	}
}

void printInfo(Info* info) {
	printf("\n========== Info ==========\n\n");
	printf("Procesy: %i\n", info->proc);
	printf("Uzytkownicy: %i\n", info->users);
	printf("Srednie obciazenie: %i\n", info->load);
	printf("Wolna pamiec: %i\n", info->freemem);
	printf("Uzyta pamiec: %i\n\n", info->totmem-info->freemem);
}

void sendInfo(int uid) {
	struct Request req;
	req.type = REQ_RETUSERINFO;
	req.id = id;
	req.value = uid;

	size_t size;
	if(listener.mode)
		size = sizeof(srv_name_in);
	else
		size = SUN_LEN(&srv_name_un);
	if(sendto(listener.socket, &req, sizeof(Request), 0, srv_name, size) == -1) {
		printf("Nie mozna wyslac\n");
		printf("%s\n", strerror(errno));
		close(listener.socket);
		exit(1);
	}

	struct sysinfo info;
	sysinfo(&info);

	int users;
	FILE *f;

	if((f = popen("who | wc -l", "r")) == NULL) {
		printf("Nie mozna pobrac informacji\n");
		printf("%s\n", strerror(errno));
		exit(1);
	}

	fscanf(f,"%d",&users);

	if(pclose(f) == -1) {
		printf("Nie mozna zamknac pliku\n");
		exit(1);
	}

	struct Info i;
	i.proc 		= info.procs;
	i.users 	= users;
	i.load 		= info.loads[2];
	i.freemem 	= info.freeram/(1024*1024);
	i.totmem 	= info.totalram/(1024*1024);


	if(sendto(listener.socket, &i, sizeof(Info), 0, srv_name, size) == -1) {
		printf("Nie mozna wyslac\n");
		printf("%s\n", strerror(errno));
		close(listener.socket);
		exit(1);
	}
}

void* startListen(void* arg) {
	socklen_t size;
	if(listener.mode)
		size = (socklen_t) sizeof(srv_name_in);
	else
		size = (socklen_t) SUN_LEN(&srv_name_un);

	int tmp;
	int err;

	for(;;) {
		if((err = pthread_mutex_lock(&mutex)) != 0) {
			printf("Nie mozna zablokowac mutexu\n");
			exit(1);
		}

		if(recvfrom(listener.socket, &tmp, sizeof(int), MSG_DONTWAIT, srv_name, &size) == -1) {
			if(errno != EAGAIN && errno != EWOULDBLOCK) {
				printf("Nie mozna odczytac\n");
				exit(1);
			}
		}
		else
			sendInfo(tmp);

		if((err = pthread_mutex_unlock(&mutex)) != 0) {
			printf("Nie mozna odblokowac mutexu\n");
			exit(1);
		}
	}
}

void listenReq(sock, mode) {
	listener.mode = mode;
	listener.socket = sock;

	int err;
	if((err = pthread_create(&thread, NULL, startListen, NULL)) != 0) {
		printf("Blad tworzenia wątku\n");
		exit(1);
	}

	if((err = pthread_detach(thread)) != 0) {
		printf("Blad tworzenia wątku\n");
		exit(1);
	}
}

void getUserInfo(int sock, int mode) {
	int key;
	scanf("%i", &key);

	Request req;
	req.type = REQ_USERINFO;
	req.value = key;
	req.id = id;

	socklen_t size;
	if(mode)
		size = (socklen_t) sizeof(srv_name_in);
	else
		size = (socklen_t) SUN_LEN(&srv_name_un);

	int err;
	if((err = pthread_mutex_lock(&mutex)) != 0) {
		printf("Nie mozna zablokowac muteksu\n");
		exit(1);
	}

	if(sendto(sock, &req, sizeof(Request), 0, srv_name, size) == -1) {
		printf("Nie mozna odebrac\n");
		printf("%s\n", strerror(errno));
		close(sock);
		exit(1);
	}

	Info info;
	if(recvfrom(sock, &info, sizeof(Info), 0, srv_name, &size) == -1) {
		printf("Nie mozna przeczytac\n");
	    exit(1);
	}

	if((err = pthread_mutex_unlock(&mutex)) != 0) {
		printf("Nie mozna zablokowac\n");
		exit(1);
	}

	printInfo(&info);

	printf("\n");
}

void work(int sock, int mode) {
	int key;

	for(;;) {
		printf("1 - Zalogowani userzy, 2 - Info o userze, 3 - Logout\n");
		fflush(stdout);

		scanf("%i", &key);

		switch(key) {
			case 1:
			{
				getLoggedUsers(sock, mode);
				break;
			}
			case 2:
			{
				getUserInfo(sock, mode);
				break;
			}
			case 3:
			{
				logout(sock, mode);
				return;
			}
		}
	}
}


void closeSession(int a) {
	logout(listener.socket, listener.mode);
	exit(1);
}


int main(int argc, char* argv[]) {
	int port = DEFAULT_PORT;
	int mode = DEFAULT_MODE;
	char address[HOST_NAME_MAX] = DEFAULT_ADDRESS;
	char file[HOST_NAME_MAX] = DEFAULT_FILE;
	int sock;

	signal(SIGINT, closeSession);

	argsParsing(argc, argv, &mode, address, &port, file);

	sock = createSocket(mode, address, port, file);

	login(sock, mode);

	listenReq(sock, mode);
	work(sock, mode);

	close(sock);

	return 0;
}
