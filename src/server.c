#include <stdio.h>
#include <stdlib.h>

#include "../include/global.h"
#include "../include/logger.h"

int socket_finder(int sock, struct user users[], int number_users) { 
	for (int i = 0; i < number_users; ++i) { 
		if (sock == users[i].socket) { 
			return i; 
		} 
	}
	return -1; 
}

void display_ip() { 
	char ip_address[16]; 
	struct sockaddr_in address; 
	int socket_fd;
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	address.sin_family = AF_INET; 
	inet_pton(AF_INET, "8.8.8.8", &address.sin_addr); 
	address.sin_port = htons(53); 
	if (connect(socket_fd, (struct sockaddr*)&address, sizeof(address)) < 0) { 
		cse4589_print_and_log("[%s:ERROR]\n", "IP");
		cse4589_print_and_log("[%s:END]\n", "IP");
		return; 
	}
	bzero(&address, sizeof(address));
	int len = sizeof(address);
	getsockname(socket_fd, (struct sockaddr *) &address, &len);
	inet_ntop(AF_INET, &address.sin_addr, ip_address, sizeof(ip_address));
	cse4589_print_and_log("[%s:SUCCESS]\n", "IP");
	cse4589_print_and_log("IP:%s\n", ip_address);
	cse4589_print_and_log("[%s:END]\n", "IP");
	close(socket_fd); 	
	return; 
}


int sending(int f_sock, char *recieving_ip, char *buffer, struct user users[], int num_users) {
	int flo = socket_finder(f_sock, users, num_users); 
	int tlo=-1;
	int j=0; 
	while(j < num_users){ 
		if (strcmp(users[j].ip_addr, recieving_ip) == 0) { 
			tlo= j; 
		}
		++j;
	}
	char error[50] = "[SEND:ERROR]\n[SEND:END]\n"; 
	if (!validate_ip(recieving_ip)) { // not a valid IP
		send(f_sock, error, strlen(error), 0);
		return 0; 
	}
	// checks local lists to see if the person trying to send to is logged in 
	int loc=-1;
	int i=0;
	while(i < users[flo].num_users){  
		if (strcmp((users[flo].storage[i])->ip_addr, recieving_ip) == 0) { 
			loc= i; 
		}
		++i;
	} 
	if (loc == -1) { 
		send(f_sock, error, strlen(error), 0);
		return 0; 
	}
	if (users[flo].storage[loc]->login_status != 1) { // user is not logged in on local list
		send(f_sock, error, strlen(error), 0);
		return 0; 
	}
	else { 
		char msg[MAXLEN], to_client[MAXLEN];	
		memset(msg, '\0', MAXLEN); 
		memset(to_client, '\0', MAXLEN); 
		int msg_len = (strlen(buffer) - (strlen("SEND") + strlen(recieving_ip) + 2));	// message prep
		int start = strlen("SEND") + 1 + strlen(recieving_ip) + 1; 
		memcpy(msg, buffer + start, msg_len); 
		msg[msg_len] = '\0'; 
		// this message sends to the client RECEIVING 
		sprintf(to_client, "[RECEIVED:SUCCESS]\nmsg from:%s\n[msg]:%s\n[RECEIVED:END]\n", users[flo].ip_addr, msg);
			if (users[tlo].login_status != 1) { 
				if (users[tlo].login_status == 0) {
					strcpy(users[tlo].buffered[users[tlo].buff_size], to_client); 
					users[tlo].buff_size++; 
					users[tlo].msg_recv++;	
				}	
			}
			else {
				send(users[tlo].socket, to_client, strlen(to_client), 0);
			}
			users[tlo].msg_recv++;	
		users[flo].msg_sent++;
		char success[50] = "[SEND:SUCCESS]\n[SEND:END]\n";		
		send(f_sock, success, strlen(success), 0); 
		cse4589_print_and_log("[RELAYED:SUCCESS]\nmsg from:%s, to:%s\n[msg]:%s\n[RELAYED:END]\n", users[flo].ip_addr, recieving_ip, msg); 
	}
}

int broadcasting(int f_sock, char *buf, struct user users[], int number_of_users) {
	int flo = socket_finder(f_sock, users, number_of_users); 
	
	int message_len = (strlen(buf) - (strlen("BROADCAST") + 1));	// message prep
	int start = strlen("BROADCAST") + 1;
	char *message = (char*) malloc(sizeof(char) * message_len); 
	memcpy(message, buf + start, message_len);
	message[message_len] = '\0'; 
	
	char to_clients[MAXLEN]; 
	sprintf(to_clients, "[RECEIVED:SUCCESS]\nmsg from:%s\n[msg]:%s\n[RECEIVED:END]\n", users[flo].ip_addr, message);
	int i=0;
	while(i < number_of_users){ 
		if (users[i].socket != f_sock) { 
			if (users[i].login_status != 1) { 
				if (users[i].login_status == 0) {
					strcpy(users[i].buffered[users[i].buff_size], to_clients); 
					users[i].buff_size++; 
				}
			}
			else { 
				send(users[i].socket, to_clients, strlen(to_clients), 0);
			}
			users[i].msg_recv++;
			users[flo].msg_sent++; 
		}		
	++i;
	}
	char success[75] = "[BROADCAST:SUCCESS]\n[BROADCAST:END]\n";
	send(f_sock, success, strlen(success), 0); 
	cse4589_print_and_log("[RELAYED:SUCCESS]\nmsg from:%s, to:%s\n[msg]:%s\n[RELAYED:END]\n", users[flo].ip_addr, "255.255.255.255", message); 
}

int validate_ip(char *ip) {   
    char t[16]; 
    memset(t, '\0', 16);
    strcpy(t, ip); 
    
    int number = 0; 
    char *args[20]; 
    
    args[number] = strtok(t, "."); 
    while (args[number] != NULL) { 
        args[++number] = strtok(NULL, "."); 
    }
    
    if (number == 4) { 
        int i=0;
        while(i < number){ 
            int j=0;
			while(j < strlen(args[i])){ 
                if (args[i][j] < '0' || args[i][j] > '9') {                 
                    return 0;
                }
            ++j;
			}
            int check = atoi(args[i]); 
            if (check > 256 || check < 0) {             
                return 0;
            }
        ++i;
		} 
    }
    else { 
		return 0;
    }
    return 1;
}

void server(char* port_char) { 
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

    int s_sock = socket(AF_INET, SOCK_STREAM, 0); // return value
	struct sockaddr_in s_ad;
	// socket()
	s_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (s_sock < 0) { 
		perror("socket() problem"); exit(-1); 
	}
	bzero(&s_ad, sizeof(s_ad));
	s_ad.sin_family = AF_INET;
	s_ad.sin_addr.s_addr = htonl(INADDR_ANY); 
	s_ad.sin_port = htons(port); 
	// bind()
	if (bind(s_sock, (struct sockaddr *)&s_ad, sizeof(s_ad)) < 0) { 
		perror("bind() problem"); exit(-1); 
	}
	// listen() 
	if (listen(s_sock, BACK) < 0) { 
		perror("listen() problem"); exit(-1); 
	}
	int server_socket = s_sock;

// for select() 
	int head_socket, selret, sock_idx, fdaccept=0, caddr_len;
	struct sockaddr_in client_addr; 
	fd_set master_list, watch_list;
	// zero FD sets
	FD_ZERO(&master_list); 
	FD_ZERO(&watch_list); 
	// register listening socket & STDIN to mastear_list
	FD_SET(server_socket, &master_list); 
	FD_SET(STDIN, &master_list); 
	
	head_socket = server_socket; 
	
// application stuff 
	struct user users[4]; 
	int num_users = 0; 
	
	while(1) { 
		memcpy(&watch_list, &master_list, sizeof(master_list)); 
		// select() system call, it'll block? 
		if ((selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL)) < 0) { 
			perror("select() problemo"); 
			exit(-1);
		} 
		else { // sockets available to process
			// loop through sockets to see which ones are ready 
			for (sock_idx = 0; sock_idx <= head_socket; ++sock_idx) { 
				if (FD_ISSET(sock_idx, &watch_list)) { 
					if (sock_idx == STDIN) { // get SERVER commands 
						char *input = (char*) malloc(sizeof(char)*MAXLEN);
						memset(input, '\0', MAXLEN);
						if(fgets(input, MAXLEN-1, stdin) == NULL) { //Mind the newline character that will be written to msg
							exit(-1);
						}
					// changing string input to array of arguments
                        int a,b;
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

                            int a,b;
                            a = -1;
                            b = 0;
                            while(cmd[b] != '\0') {
                                if(cmd[b] != ' ' && cmd[b] != '\t' && cmd[b] != '\n') {
                                    a= b;
                                }
                                b++;
                            }
                            cmd[a + 1] = '\0';

							if (num_args == 1) { 
								args[0][strlen(args[0])] = '\0';
							}
						}
						if (strcmp(cmd, "AUTHOR") == 0) { 
							cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
							cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", "rishijay");
							cse4589_print_and_log("[%s:END]\n", cmd);
						}
						else if (strcmp(cmd, "IP") == 0) { display_ip(); }
						else if (strcmp(cmd, "PORT") == 0) {
                            cse4589_print_and_log("[%s:SUCCESS]\n", "PORT");
                            cse4589_print_and_log("PORT:%d\n", 	port);
                            cse4589_print_and_log("[%s:END]\n", "PORT");	
                         }
						else if (strcmp(cmd, "LIST") == 0) {
                            for (int i = 0; i < num_users; ++i) { 
                                int min = i; 
                                for (int j = i+1; j < num_users; ++j) { 
                                    if (users[j].port < users[min].port) { 
                                        min = j; 
                                    }
                                }
                                struct user temp = users[i];
                                users[i] = users[min]; 
                                users[min] = temp; 
                            }
							int x = 1;
							cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
							for (int i = 0; i < num_users; ++i) { 
								if (users[i].login_status == 1) { 
									cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", x, users[i].host_id, users[i].ip_addr, users[i].port); 
									++x;
								}
							}
							cse4589_print_and_log("[%s:END]\n", cmd);
						}
						else if (strcmp(cmd, "STATISTICS") == 0) {
							// sort by port 
                            for (int i = 0; i < num_users; ++i) { 
                                int min = i; 
                                for (int j = i+1; j < num_users; ++j) { 
                                    if (users[j].port < users[min].port) { 
                                        min = j; 
                                    }
                                }
                                struct user temp = users[i];
                                users[i] = users[min]; 
                                users[min] = temp; 
                            }
							// print out statistics
							cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
							for (int i = 0; i < num_users; ++i) {
								if (users[i].login_status == 1 || users[i].login_status == 0) { 
									char *status; 
									if (users[i].login_status == 1) { 
										status = "logged-in"; 
									}
									else if (users[i].login_status == 0) { 
										status = "logged-out"; 
									}
									else { /* do nothing, unidentified login_status */ }
									cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", i+1, users[i].host_id, users[i].msg_sent, users[i].msg_recv, status);
								}
							} 
							cse4589_print_and_log("[%s:END]\n", cmd);
						}
						else if (strcmp(cmd, "BLOCKED") == 0) { 
							char success[50] = "[BLOCKED:SUCCESS]\n[BLOCKED:END]\n"; 
							char error[50] = "[BLOCKED:ERROR]\n[BLOCKED:END]\n";
                            int a=-1;
                            for (int i = 0; i < num_users; ++i) { 
                                if (strcmp(users[i].ip_addr, args[1]) == 0) { 
                                    a= i;
                                    break; 
                                }
                            }
							if (!validate_ip(args[1]) || (a == -1)) { 
								cse4589_print_and_log(error); 
							}
							else {
                                int loc=-1;
                                    for (int i = 0; i < num_users; ++i) { 
                                        if (strcmp(users[i].ip_addr, args[1]) == 0) { 
                                            loc=i; 
                                        }
                                    }
								cse4589_print_and_log("[BLOCKED:SUCCESS]\n"); 
								for (int i = 0; i < users[loc].number_blocked; ++i) {
									cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", i, users[loc].blocked[i]->host_id, users[loc].blocked[i]->ip_addr, users[loc].blocked[i]->port); 									
								}
								cse4589_print_and_log("[BLOCKED:END]\n"); 
							}
							
						}
						else { /* no other input should need to be handled specifically */ }
					}
					else if (sock_idx == server_socket) { // handle new clients
						caddr_len = sizeof(client_addr); 
						if ((fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len)) < 0) { 
							perror("accept() on incoming client failed");
						}
						else { // client logged in 						
							char ip_addr[16]; 
							inet_ntop(AF_INET, &client_addr.sin_addr, ip_addr, sizeof(ip_addr));
							int loc=-1; 
                                for (int i = 0; i < num_users; ++i) { 
                                    if (strcmp(users[i].ip_addr, ip_addr) == 0) { 
                                        loc= i; 
                                    }
                                } 
							int logged_in_before = 0; 
							if (loc > -1)  {
								logged_in_before = 1; 
							} 
							if (logged_in_before) { 
								users[loc].login_status = 1; // log user back in
								users[loc].socket = fdaccept; // update to current connected socket
							} 
							else { // has not logged in before or EXITed client
								struct user new_user;
								memset(new_user.host_id, '\0', 35); 
								if (strcmp(ip_addr, "128.205.36.33") == 0) { strcpy(new_user.host_id, "highgate.cse.buffalo.edu"); }
								else if (strcmp(ip_addr, "128.205.36.34") == 0) { strcpy(new_user.host_id, "euston.cse.buffalo.edu"); }
								else if (strcmp(ip_addr, "128.205.36.35") == 0) { strcpy(new_user.host_id, "embankment.cse.buffalo.edu"); }
								else if (strcmp(ip_addr, "128.205.36.46") == 0) { strcpy(new_user.host_id, "stones.cse.buffalo.edu"); }
								else if (strcmp(ip_addr, "128.205.36.36") == 0) { strcpy(new_user.host_id, "underground.cse.buffalo.edu");	}
								else { strcpy(new_user.host_id, "?????.cse.buffalo.edu"); }  // unidentified server							
								strcpy(new_user.ip_addr, ip_addr); 
								new_user.port = ntohs(client_addr.sin_port); 
								new_user.login_status = 1; 
								new_user.socket = fdaccept; 							
								new_user.msg_sent = 0; 						
								new_user.msg_recv = 0;
								new_user.buff_size = 0; 
								new_user.number_blocked = 0; 
								users[num_users] = new_user; 
								num_users++;
							}
							// add back into the list of sockets to watch
							FD_SET(fdaccept, &master_list); 
							if (fdaccept > head_socket) { 
								head_socket = fdaccept; 
							}
							// save current LIST in storage for future SEND, BLOCK, UNBLOCK comands
                            loc=-1;
                                for (int i = 0; i < num_users; ++i) { 
                                    if (strcmp(users[i].ip_addr, ip_addr) == 0) { 
                                        loc= i; 
                                        break;
                                    }
                                } 
							for (int i = 0; i < num_users; ++i) { 
								users[loc].storage[i] = &users[i]; 
							}		
							users[loc].num_users = num_users; 
							// sends the user the current list - 
							char *temp[num_users];

                                // sort items by port
                                for (int i = 0; i < num_users; ++i) { 
                                    int min = i; 
                                    for (int j = i+1; j < num_users; ++j) { 
                                        if (users[j].port < users[min].port) { 
                                            min = j; 
                                        }
                                    }
                                    struct user t = users[i];
                                    users[i] = users[min]; 
                                    users[min] = t; 
                                }
                                int cnt = 1; 
                                for (int i = 0; i < num_users; ++i) {
                                    if (users[i].login_status == 1) {		// users logged in
                                        char *buffer = (char*) malloc(sizeof(char)*MAXLEN);
                                        memset(buffer, '\0', MAXLEN);
                                        sprintf(buffer, "%-5d%-35s%-20s%-8d\n", cnt, users[i].host_id, users[i].ip_addr, users[i].port); 
                                        temp[cnt-1] = buffer; 
                                        ++cnt; 
                                    }
                                }

							char *sendable = (char*) malloc(sizeof(char) * MAXLEN); 
							memset(sendable, '\0', MAXLEN); 
							for (int i = 0; i < num_users; ++i) { 
								strcat(sendable, temp[i]); 
							}
							send (fdaccept, sendable, strlen(sendable), 0);
							// sends the first section of LOGIN response messages
							char success[50] = "[LOGIN:SUCCESS]\n";
							send (fdaccept, success, strlen(success), 0); 
							// sends the user the buffered messages - 
							if (logged_in_before) { 
								for (int i = 0; i < users[loc].buff_size; ++i) { 
									send(fdaccept, users[loc].buffered[i], strlen(users[loc].buffered[i]), 0);						
								}
								users[loc].buff_size = 0; 
							}
							// sends second half of LOGIN response message
							char end[50] = "[LOGIN:END]\n"; 
							send(fdaccept, end, strlen(end), 0);
						}
					}
					else { // handle current clients' inputs
						char *buffer = (char*) malloc(sizeof(char)*MAXLEN);
						memset(buffer, '\0', MAXLEN);
						if (recv(sock_idx, buffer, MAXLEN, 0) <= 0) {	// client disconnected
							int remove = socket_finder(sock_idx, users, num_users); 
							if (remove != -1) { 
								for (int i = remove; i < num_users; ++i) {
									users[i] = users[i+1];
								}
								num_users--; 
								close(sock_idx); 
								FD_CLR(sock_idx, &master_list); 
							}
						}
						else { 
							// convert string to args and set cmd  

                            int a,b;
                            a = -1;
                            b = 0;
                            while(buffer[b] != '\0') {
                                if(buffer[b] != ' ' && buffer[b] != '\t' && buffer[b] != '\n') {
                                    a= b;
                                }
                                b++;
                            }
                            buffer[a + 1] = '\0';

							char *temp = (char*) malloc(sizeof(char)*strlen(buffer)); 
							strcpy(temp, buffer); 
							
							char *args[100]; // maximum of 100 words allowed
							int num_args = 0; 
							args[num_args] = strtok(temp, " "); 
							while (args[num_args] != NULL) { 
								args[++num_args] = strtok(NULL, " "); 
							}
							char *cmd = (char*) malloc(sizeof(char)*CMDSIZE);
							memset(cmd, '\0', CMDSIZE); strcpy(cmd, args[0]); 

                            int c,d;
                            c = -1;
                            d = 0;
                            while(cmd[d] != '\0') {
                                if(cmd[d] != ' ' && cmd[d] != '\t' && cmd[d] != '\n') {
                                    c=d;
                                }
                                d++;
                            }
                            cmd[c + 1] = '\0';

							// process client commands
							if (strcmp("LOGOUT", cmd) == 0) {
								int loc = socket_finder(sock_idx, users, num_users); 
								users[loc].login_status = 0; 
								close(sock_idx); 
								FD_CLR(sock_idx, &master_list);
							}
							else if (strcmp("EXIT", cmd) == 0) {
								int loc = socket_finder(sock_idx, users, num_users); 
								for (int i = loc; i < num_users; ++i) {
									users[i] = users[i+1]; 
								}
								num_users--; 
								close(sock_idx); 
								FD_CLR(sock_idx, &master_list);	
							}
							else if (strcmp("REFRESH", cmd) == 0) {
								// fetch list, place into a buffer
								char *temp[num_users];

                                    // sort items by port
                                    for(int i = 0; i < num_users; ++i) { 
                                        int min = i; 
                                        for (int j = i+1; j < num_users; ++j) { 
                                            if (users[j].port < users[min].port) { 
                                                min = j; 
                                            }
                                        }
                                        struct user t = users[i];
                                        users[i] = users[min]; 
                                        users[min] = t; 
                                    }
                                    int cnt = 1; 
                                    for (int i = 0; i < num_users; ++i) {
                                        if (users[i].login_status == 1) {		// users logged in
                                            char *buffer = (char*) malloc(sizeof(char)*MAXLEN);
                                            memset(buffer, '\0', MAXLEN);
                                            sprintf(buffer, "%-5d%-35s%-20s%-8d\n", cnt, users[i].host_id, users[i].ip_addr, users[i].port); 
                                            temp[cnt-1] = buffer; 
                                            ++cnt; 
                                        }
                                    }

								char *list = (char*) malloc(sizeof(char) * MAXLEN); 
								memset(list, '\0', strlen(list)); 
								for (int i = 0; i < num_users; ++i) { 
									strcat(list, temp[i]); 
								}
								// send out
								send(sock_idx, list, strlen(list), 0); 
								// save list in user files
								int loc = socket_finder(sock_idx, users, num_users);
								for (int i = 0; i < num_users; ++i) { 
									users[loc].storage[i] = &users[i]; 
								} 
								users[loc].num_users = num_users; 
								char success[50] = "[REFRESH:SUCCESS]\n[REFRESH:END]\n"; 
								send(sock_idx, success, strlen(success), 0); 
							}
							else if (strcmp("SEND", cmd) == 0) {
								sending(sock_idx, args[1], buffer, users, num_users); 
							}
							else if (strcmp("BROADCAST", cmd) == 0) {
                                broadcasting(sock_idx, buffer, users, num_users);
							}
							else { /* do nothing, command not recognized or handled in client */ }
						}
					} 					
				}		
				
			}
		}	
	}		
}