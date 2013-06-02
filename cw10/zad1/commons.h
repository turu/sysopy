#ifndef _COMMONS_H_
#define _COMMONS_H_

#define DEFAULT_ADDRESS "127.0.0.1"
#define DEFAULT_FILE "/tmp/server"
#define HOST_NAME_MAX 60
#define DEFAULT_PORT (uint16_t)5432
#define DEFAULT_MODE 1
#define REQ_LOGIN 0
#define REQ_GETUSERS 1
#define REQ_USERINFO 2
#define REQ_LOGOUT 3
#define REQ_RETUSERINFO 4
#define REQ_EXECUTE 5
#define REQ_RESP_EXECUTE 6
#define MAXCLIENTS 100
#define MODE_UNIX 0
#define MODE_INET 1

typedef struct UserInfoHeader {
	int count;
} UserInfoHeader;

typedef struct User {
	int id;
	int mode;
	size_t size;
	char name[100];
	struct sockaddr* client_name;
} User;

typedef struct Request {
	int mode;
	size_t size;
	int type;
	int id;
	int value;
	char name[100];
} Request;

typedef struct Listener {
	int socket;
	int mode;
} Listener;

typedef struct Info {
	int proc;
	int users;
	int load;
	int freemem;
	int totmem;
} Info;

typedef struct Result {

} Result;

#endif
