#ifndef _SERVER_H_
#define _SERVER_H_

#include <netinet/in.h>

extern MAX_SESSIONS

// one session (game) with two players
struct session {
    int player1_fd;
    int player2_fd;
    pid_t pid;
    int active;
}

struct sockaddr_in *init_server_addr(int port);
int set_up_server_socket(struct sockaddr_in *self, int num_queue);
int accept_connection(int listenfd);

#endif