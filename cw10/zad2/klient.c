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

#include "structures.h"

int id;
int sock;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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
	struct sockaddr_in srv_name_in;
	int csock;
	
	if((csock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
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
    
    	if(connect(csock, (struct sockaddr*)&srv_name_in, sizeof(srv_name_in)) == -1) {
    		printf("Nie mozna polaczyc\n");
        	exit(1);
    	}
    	
    return csock;
}


int createSocketUNIX(char* file) {
	struct sockaddr_un srv_name_un;
	int csock;
	
	if((csock = socket(PF_UNIX, SOCK_STREAM, 0)) == -1) {
        	printf("Nie mozna otworzyc soketu\n");
       		exit(1);
    	}
    
    	srv_name_un.sun_family = AF_UNIX;
    	strcpy(srv_name_un.sun_path, file);
    
    	if(connect(csock, (struct sockaddr*)&srv_name_un, SUN_LEN(&srv_name_un)) == -1) {
    		printf("%s\n", strerror(errno));
    		printf("Nie mozna polaczyc\n");
        	exit(1);
    	}
    	
    return csock;
}

int createSocket(int mode, char address[], int port, char file[]) {
	return mode ? createSocketINTERNET(address, port) : createSocketUNIX(file);
}

void getLoggedUsers() {
	struct Request req;
	req.type = REQ_LOGGEDUSERS;
	req.id = id;
	
	if(send(sock, &req, sizeof(struct Request), 0) == -1) {
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
	
	struct PendingMessages pm;
	if(recv(sock, &pm, sizeof(struct PendingMessages), 0) == -1) {
		printf("Nie mozna czytac wiadomosci\n");
	    exit(1);
	}
	
	printf("========== UZYTKOWNICY ==========\n\n");
	
	struct LoggedUser lu;
	int i;
	for(i = 0; i < pm.count; ++i) {
		if(recv(sock, &lu, sizeof(struct LoggedUser), 0) == -1) {
			printf("Nie mozna czytac wiadomosci\n");
			exit(1);
		}
		
		printf("ID: %i   %s\n", lu.id, lu.name);
	}
	
	printf("\n");
	
	if((err = pthread_mutex_unlock(&mutex)) != 0) {
		printf("Nie mozna zablokowac mutexu\n");
		exit(1);
	}
}

void login() {	
	struct Request lg;
	gethostname(lg.name, sizeof(lg.name));
	sprintf(lg.name, "%s : %s", lg.name,  gethostent()->h_name);
	lg.id = -1;
	lg.type = REQ_LOGIN;
	
	if(send(sock, &lg, sizeof(struct Request), 0) == -1) {
		printf("Nie mozna zalogowac, %s, soket %i\n", lg.name, sock);
		printf("%s\n", strerror(errno));
		close(sock);
		exit(1);
	}
	
	if(recv(sock, &id, sizeof(int), 0) == -1) {
		printf("Nie mozna odczytac id\n");
		exit(1);
	}
	
	getLoggedUsers();
}

void logout() {	
	struct Request lg;
	gethostname(lg.name, sizeof(lg.name));
	sprintf(lg.name, "%s : %s", lg.name,  gethostent()->h_name);
	lg.id = id;
	lg.type = REQ_LOGOUT;
	
	if(send(sock, &lg, sizeof(struct Request), 0) == -1) {
		printf("%s\n", strerror(errno));
		close(sock);
		exit(1);
	}
}

void printInfo(struct Info* info) {
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
	
	if(send(sock, &req, sizeof(struct Request), 0) == -1) {
		printf("Nie mozna wyslac\n");
		printf("%s\n", strerror(errno));
		close(sock);
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
	i.proc = info.procs;
	i.users = users;
	i.load = info.loads[2];
	i.freemem = info.freeram/(1024*1024);
	i.totmem = info.totalram/(1024*1024);
	
	
	if(send(sock, &i, sizeof(struct Info), 0) == -1) {
		printf("Nie mozna wyslac informacji\n");
		printf("%s\n", strerror(errno));
		close(sock);
		exit(1);
	}
}

void* startListen(void* arg) {
	int tmp;
	int err;
		
	for(;;) {
		if((err = pthread_mutex_lock(&mutex)) != 0) {
			printf("Nie mozna zablokowac muteksu\n");
			exit(1);
		}
		
		if(recv(sock, &tmp, sizeof(int), MSG_DONTWAIT) == -1) {
			if(errno != EAGAIN && errno != EWOULDBLOCK) {
				printf("Nie mozna odczytac id\n");
				exit(1);
			}
		}	
		else
			sendInfo(tmp);
		
		if((err = pthread_mutex_unlock(&mutex)) != 0) {
			printf("Nie mozna odblokowac muteksu\n");
			exit(1);
		}
	}
}

void listenReq() {
	pthread_t thread;
	int err;
	if((err = pthread_create(&thread, NULL, startListen, NULL)) != 0) 	{
		printf("Nie mozna utworzyc watku\n");
		exit(1);
	}
	
	if((err = pthread_detach(thread)) != 0) {
		printf("Nie mozna utworzyc watku\n");
		exit(1);
	}
}

void getUserInfo() {
	int key;
	scanf("%i", &key);
	
	struct Request req;
	req.type = REQ_USERINFO;
	req.value = key;
	req.id = id;
		
	int err;
	if((err = pthread_mutex_lock(&mutex)) != 0) {
		printf("Nie mozna zablokowac mutexu.\n");
		exit(1);
	}
		
	if(send(sock, &req, sizeof(struct Request), 0) == -1) {
		printf("Nie mozna wyslac\n");
		printf("%s\n", strerror(errno));
		close(sock);
		exit(1);
	}
	
	struct Info info;
	if(recv(sock, &info, sizeof(struct Info), 0) == -1) {
		printf("Nie mozna odczytac\n");
	    exit(1);
	}
	
	if((err = pthread_mutex_unlock(&mutex)) != 0) {
		printf("Nie mozna zablkowac muteksu\n");
		exit(1);
	}
	
	printInfo(&info);
	
	printf("\n");
}

void work() {
	int key;
	
	for(;;) {
		printf("1 - Zalogowani userzy, 2 - Info o userze, 3 - Logout\n");
		fflush(stdout);
		
		scanf("%i", &key);
		
		switch(key) {
			case 1:
			{
				getLoggedUsers();
				break;
			}
			case 2:
			{
				getUserInfo();
				break;
			}
			case 3:
			{
				logout();
				return;
			}
		}
	}
}


void closeSession(int a) {
	logout();
	exit(1);
}


int main(int argc, char* argv[]) {
	int port = DEFAULT_PORT;
	int mode = DEFAULT_MODE;
	char address[HOST_NAME_MAX] = DEFAULT_ADDRESS;
	char file[HOST_NAME_MAX] = DEFAULT_FILE;
	
	signal(SIGINT, closeSession);
	
	argsParsing(argc, argv, &mode, address, &port, file);
	
	sock = createSocket(mode, address, port, file);
	
	login();
	
	listenReq();
	work();
	
	close(sock);
	
	return 0;
}
