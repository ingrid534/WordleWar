#ifndef _SERVER_H_
#define _SERVER_H_

#include <sys/types.h>
#include <netinet/in.h>
#include "player.h"

// one session (game) with two players
struct session {
    Player *player1;
    Player *player2;
    pid_t pid;
    int active;
};

struct sockaddr_in *init_server_addr(int port);
int set_up_server_socket(struct sockaddr_in *self, int num_queue);
int accept_connection(int listenfd);

#endif