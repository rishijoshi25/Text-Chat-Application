#include <stdio.h>
#include <stdlib.h>

#include "../include/global.h"
#include "../include/logger.h"

void client(char* port_char) { 
	int o=1;
    for (int i = 0; i < strlen(port_char); ++i) { 
		if ((port_char[i] < '0' || port_char[i] > '9') && port_char[i] > -1) { 
			o=0; 
		}
	}
	if (!o) { 
		perror ("not a valid port!"); 
		return; 
	}
	int port = atoi(port_char); 
	int server = -1;
	char *list_storage = (char*) malloc(sizeof(char)*1000); // string that holds LIST information. updated with REFRESH
	memset(list_storage, '\0', 1000); 
	int refresh = 0, login = 0; 
	int selret, sock_idx, head_socket; 
	fd_set watch_list, master_list;  
	// zero FD sets
	FD_ZERO(&master_list); 
	FD_ZERO(&watch_list); 
	// register STDIN to master_list
	FD_SET(STDIN, &master_list); 
	head_socket = STDIN; 
	
	while(1) { 		
		memcpy(&watch_list, &master_list, sizeof(master_list)); 
		// select() system call, it'll block? 
		if ((selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL)) < 0) { 
			perror("select() problem"); 
			exit(-1);
		} 
		else { // sockets available to process
			// loop through sockets to see which ones are ready 
			for (sock_idx = 0; sock_idx <= head_socket; ++sock_idx) { 
				if (FD_ISSET(sock_idx, &watch_list)) { 
					if (sock_idx == STDIN) { // get client input
						char *input = (char*) malloc(sizeof(char)*MAXLEN);
						memset(input, '\0', MAXLEN);
						if(fgets(input, MAXLEN-1, stdin) == NULL) { //Mind the newline character that will be written to msg
							exit(-1);
						}
					// changing string input to array of arguments
						int a, b;
						a = -1;
						b = 0;
						while(input[b] != '\0') {
							if(input[b] != ' ' && input[b] != '\t' && input[b] != '\n') {
								a= b;
							}
							b++;
						}
						input[a + 1] = '\0';
						
						char *cmd = (char*) malloc(sizeof(char)*CMDSIZE);
						memset(cmd, '\0', CMDSIZE); 
						int num_args = 0; 
						char *args[100]; // maximum of 100 words allowed
						if (strcmp("", input) != 0) { 
							char *temp = (char*) malloc(sizeof(char)*strlen(input)); 
							strcpy(temp, input); 
							args[num_args] = strtok(temp, " "); 
							while (args[num_args] != NULL) { 
								args[++num_args] = strtok(NULL, " "); 
							}
							strcpy(cmd, args[0]); 
							int c, d;
							c = -1;
							d = 0;
							while(cmd[d] != '\0') {
								if(cmd[d] != ' ' && cmd[d] != '\t' && cmd[d] != '\n') {
									c= d;
								}
								d++;
							}
							cmd[c + 1] = '\0';
							if (num_args == 1) { 
								args[0][strlen(args[0])] = '\0';
							}
//							free(temp); 
						}						 
					// process commands	given by client	
						if (strcmp(cmd, "AUTHOR") == 0) { 
							cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", "rishijay");
						}
						else if (strcmp(cmd, "IP") == 0) { 
							display_ip(); 
							}
						else if (strcmp(cmd, "PORT") == 0) { 
							cse4589_print_and_log("[%s:SUCCESS]\n", "PORT");
						cse4589_print_and_log("PORT:%d\n", 	port);
						cse4589_print_and_log("[%s:END]\n", "PORT");	
     }
						else if (strcmp(cmd, "LOGIN") == 0) {
							if (num_args == 3) { 
								server = host_connection(args[1], args[2], port); 
								if (server > -1) { 
									FD_SET(server, &master_list); 
									if (server > head_socket) { 
										head_socket = server; 
									} 
									login = 1; 
								} 
								else { 
									cse4589_print_and_log("[%s:ERROR]\n", cmd);
									cse4589_print_and_log("[%s:END]\n", cmd);
								}
							}
						}
						else if (strcmp(cmd, "EXIT") == 0) {
							if (server == -8) { // previously logged out?? server should be set to -8
								server = host_connection(args[1], args[2], port); 
							}
							if (server > 0) { 
								send(server, cmd, strlen(cmd), 0); 
								// let server disconnect from client
							}
							cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
							cse4589_print_and_log("[%s:END]\n", cmd);
							exit(0); 
						}
						else { // server handled commands
							if (server > 0) { 
								if (strcmp("LIST", cmd) != 0) {
									send(server, input, strlen(input), 0); // communicate with server 
									if (strcmp("LOGOUT", cmd) == 0) {															
										FD_CLR(server, &master_list); 
										close(server); 
										server = -8; 
										cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
										cse4589_print_and_log("[%s:END]\n", cmd);
									}
									else if (strcmp("REFRESH", cmd) == 0) { 
										refresh = 1; 
									}
									else { /* do nothing, everything else gets sent to the server */ }
								}
								else { 
									cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
									cse4589_print_and_log("%s", list_storage); 
									cse4589_print_and_log("[%s:END]\n", cmd);
								}		
							}
						}
					}
					else if (sock_idx == server) { 
						char *buffer = (char*) malloc(sizeof(char)*MAXLEN);
						memset(buffer, '\0', MAXLEN);
						if (recv(server, buffer, MAXLEN, 0) <= 0) { 
							FD_CLR(sock_idx, &master_list); 
							close(server); 
						}
						else {
							if (!refresh && !login) { 
								cse4589_print_and_log("%s", buffer); 
								fflush(stdout); 
							}
							if (refresh || login) {
								strcpy(list_storage, buffer); 
								refresh = 0;
								login = 0; 
							}
						}
					}
					else { /* do nothing */ }
				}
			}
		}
	}
}


int host_connection(char *s_ip, char *spc, int h_p) {
	int f=1;
	int i=0;
	while(i < strlen(spc)){ 
		if ((spc[i] < '0' || spc[i] > '9') && spc[i] > -1) { 
			f=0; 
		}
		++i;
	} 

	if (validate_ip(s_ip) && f) { 
		int s_p = atoi(spc); 
		int sockfd; 
		struct sockaddr_in s_address, cl_address;

		sockfd = socket(AF_INET, SOCK_STREAM, 0); 
		if (sockfd < 0) { 
			perror("socket() failed\n"); 
		}
		bzero(&cl_address, sizeof(cl_address)); 
		cl_address.sin_family = AF_INET;
		cl_address.sin_addr.s_addr = htonl(INADDR_ANY); 
		cl_address.sin_port = htons(h_p); 
		if (bind(sockfd, (struct sockaddr *) &cl_address, sizeof(struct sockaddr_in)) != 0) { 
			perror("failed to bind port to client"); 
		}
		
		bzero(&s_address, sizeof(s_address));
		s_address.sin_family = AF_INET;
		inet_pton(AF_INET, s_ip, &s_address.sin_addr);
		s_address.sin_port = htons(s_p);

		if(connect(sockfd, (struct sockaddr*)&s_address, sizeof(s_address)) < 0) {
			sockfd = -1; 
			cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
			cse4589_print_and_log("[%s:END]\n", "LOGIN");
			return -1; 
		}
		return sockfd;
	}
	else { 
		cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
		cse4589_print_and_log("[%s:END]\n", "LOGIN");
		return -1;
	}
    
}