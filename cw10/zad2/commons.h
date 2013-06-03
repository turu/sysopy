#ifndef _COMMONS_H_
#define _COMMONS_H_

#define DEFAULT_ADDRESS "127.0.0.1"
#define DEFAULT_FILE "server"
#define HOST_NAME_MAX 120
#define MODE_UNIX 0
#define MODE_INET 1
#define DEFAULT_PORT (uint16_t)5432
#define DEFAULT_MODE MODE_UNIX
#define REQ_LOGIN 0
#define REQ_GETUSERS 1
#define REQ_LOGOUT 2
#define REQ_EXECUTE 3
#define REQ_RESP_EXECUTE 4
#define MAXCLIENTS 100

typedef struct UserInfoHeader {
	int user_count;
} UserInfoHeader;

typedef struct User {
	int id;
	char name[100];
	int sock;
} User;

typedef struct Request {
	int type;
	int id;
	int value;
	char name[100];
} Request;

typedef struct Listener {
	int socket;
	int mode;
} Listener;

typedef struct CommandRequest {
    int callerId;
    int targetId;
    char command_name[100];
    char value[256];
} CommandRequest;

#endif
