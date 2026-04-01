#ifndef _SERVER_H_
#define _SERVER_H_

#include <sys/types.h>
#include <netinet/in.h>
#include "player.h"

struct sockaddr_in *init_server_addr(int port);
int set_up_server_socket(struct sockaddr_in *self, int num_queue);
int accept_connection(int listenfd);
void write_to_client(int client_fd, const char *msg);
int find_network_newline(const char *buf, int n);
void read_client_msg(Player *player);
char *extract_msg(Player *player);
char *generate_msg(const char *a, const char *b);
void write_status_with_score(int client_fd, const char *status_prefix, int score);
void handle_choice(int client_fd, char *choice);
void handle_player(int client_fd);

#endif