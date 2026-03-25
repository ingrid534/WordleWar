#ifndef _SERVER_H_
#define _SERVER_H_

#include <netinet/in.h>

struct sockaddr_in *init_server_addr(int port);
int set_up_server_socket(struct sockaddr_in *self, int num_queue);
int accept_connection(int listenfd);

#endif