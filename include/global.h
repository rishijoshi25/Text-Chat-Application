#ifndef GLOBAL_H_
#define GLOBAL_H_

#define HOSTNAME_LEN 128
#define PATH_LEN 256

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <limits.h> 

#define MAXLEN 500
#define CMDSIZE 50
#define BACK 5
#define STDIN 0
struct user {
	char host_id[35]; 
	int msg_sent; 
	int msg_recv; 
	
	char buffered[100][MAXLEN]; 
	int buff_size;
	char ip_addr[16]; 
	int port; 
	int login_status; // -1=disconnected, 0=logged-out, 1=logged-in,
	int socket;   
	
	int number_blocked; 
	struct user *blocked[4];
	
	struct user *storage[4];
	int num_users; 
};

#endif
