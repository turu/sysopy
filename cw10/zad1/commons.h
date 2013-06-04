#ifndef _COMMONS_H_
#define _COMMONS_H_

#define MODE_UNIX 0
#define MODE_INET 1
#define REQ_LOGIN 0
#define REQ_LOGOUT 2
#define REQ_EXECUTE 3
#define REQ_GETUSERS 1
#define MAXCLIENTS 100
#define REQ_RESP_EXECUTE 4
#define DEFAULT_ADDRESS "127.0.0.1"
#define DEFAULT_PORT (uint16_t)5432
#define DEFAULT_MODE MODE_UNIX
#define DEFAULT_FILE "server"
#define HOST_NAME_MAX 120

typedef struct UserInfoHeader {
	int user_count;
} UserInfoHeader;

typedef struct User {
	int id;
	int mode;
	size_t size;
	char name[100];
	struct sockaddr* client_name;
} User;

typedef struct Request {
	int id;
	int mode;
	int type;
	int value;
	size_t size;
	char name[100];
} Request;

typedef struct Listener {
	int socket;
	int mode;
} Listener;

typedef struct CommandRequest {
    int callerId;
    int targetId;
    char value[256];
    char command_name[100];
} CommandRequest;

#endif
