#ifndef _STRUCTURES_H_
#define _STRUCTURES_H_

#define DEFAULT_ADDRESS "127.0.0.1"
#define DEFAULT_FILE "/tmp/server"
#define HOST_NAME_MAX 60
#define DEFAULT_PORT (uint16_t)5555
#define DEFAULT_MODE 1
#define REQ_LOGIN 0
#define REQ_LOGGEDUSERS 1
#define REQ_USERINFO 2
#define REQ_LOGOUT 3
#define REQ_RETUSERINFO 4
#define LOGGEDMAX 100

struct PendingMessages {
	int count;
};

struct LoggedUser {
	int id;
	char name[100];
	int socket;
};

struct Request {
	int type;
	int id;
	int value;
	char name[100];
};

struct Listener {
	int socket;
	int mode;
};

struct Info {
	int proc;
	int users;
	int load;
	int freemem;
	int totmem;
};

#endif
