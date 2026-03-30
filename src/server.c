#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>    /* Internet domain header */
#include <arpa/inet.h>   /* inet_ntoa() - might only need on mac */ 

#include "_SERVER_H_"
#include "PLAYER_H"
#include "GAME_H"
#include "PROTOCOL_H"

#define MAX_SESSIONS 50;

Player *clients[FD_SETSIZE] = {NULL};

/*
 * Initialize a server address associated with the given port.
 */
struct sockaddr_in *init_server_addr(int port) {
    struct sockaddr_in *addr = malloc(sizeof(struct sockaddr_in));

    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    memset(&(addr->sin_zero), 0, 8);

    addr->sin_addr.s_addr = INADDR_ANY;

    return addr;
}

/*
* Create and set up a socket for a server to listen on
*/
int set_up_server_socket(struct sockaddr_in *self, int num_queue) {
    int listen_soc = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_soc == -1) {
        perror("server: socket");
        exit(1);
    }
    
    // avoid address in use error (see lab 10 socket.c)
    int on = 1;
    int status = setsockopt(soc, SOL_SOCKET, SO_REUSEADDR,
                            (const char *) &on, sizeof(on));
    if (status < 0) {
        perror("setsockopt");
        exit(1);
    }

    // bind socket to addr
    if (bind(listen_soc, (struct sockadd *) self, sizeof(struct sockaddr_in)) == -1) {
        perror("server: bind");
        exit(1);
    }

    // set up queue for pending connections
    if (listen(listen_soc, num_queue) < 0) {
        perror("server: listen");
        exit(1);
    }

    return listen_soc;
}

/*
* Accept connection from client
*/
int accept_connection(int listenfd) {
    // initialize client address
    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    unsigned int client_len = sizeof(struct sockaddr_in);

    // accept connection
    int client_soc = accept(listenfd, (struct sockaddr *) &client_addr, &client_len);
    if (client_soc < 0) {
        perror("server: accept");
        exit(1);
    }

    return client_soc;
}

void write_to_client(int client_fd, char *msg) {
    char buf[1024];
    sprintf(buf, "%s\r\n", msg);

    if (write(client_fd, msg, strlen(buf)) == -1) {
        perror("server: write");
        exit(1);
    }
}

/*
 * Search the first n characters of buf for a network newline (\r\n).
 * Return one plus the index of the '\n' of the first network newline,
 * or -1 if no network newline is found. The return value is the index into buf
 * where the current line ends.
 * Definitely do not use strchr or other string functions to search here. (Why not?)
 */
int find_network_newline(const char *buf, int n) {
	for (int i = 0; i < n-1; i++) {
		if (buf[i] == '\r' && buf[i+1] == '\n') {
			return i + 2;
		}
	}
    return -1;
}

/*
* Read message from client, calling read only
* once to avoid server blocking on client.
*/
void read_client_msg(Player *player) {
    // receive message (with buffer from player struct)
    int room = sizeof(player->buf) - player->inbuf;
    char *after = &player->buf[player->inbuf];
    int nbytes = read(player->fd, after, room);
    
    if (nbytes == 0) {
        printf("Client %d disconnected.\n", player->fd);
        // TODO: handle player disconnect here (close fd, free memory, etc.)
        return;
    } else if (nbytes < 0) {
        perror("server: read");
        return;
    }
    player->inbuf += nbytes;
}


/*
* Extract a complete message out of player's buffer if one is ready.
* Otherwise, return NULL.
*/
char *extract_msg(Player *player) {
    int where;
    while ((where = find_network_newline(player->buf, player->inbuf)) > 0) {
        // null-terminate the string to extract the message
        player->buf[where - 2] = '\0'; 
        
        // Make copy of msg to process it
        char *msg = malloc(where - 1); 
        strcpy(msg, player->buf);

        player->inbuf -= where;
        memmove(player->buf, &player->buf[where], player->inbuf);

        return msg;
    }
    return NULL;
}

/*
* Concatenate the two strings into a new message.
*/
char *generate_msg(char *a, char *b) {
    if (a == NULL || b == NULL) {
        return NULL; 
    }

    size_t total_length = strlen(a) + strlen(b) + 1;

    char *msg = malloc(total_length);
    if (msg == NULL) {
        perror("server: malloc failed in generate_msg");
        exit(1); 
    }

    strcpy(msg, a);
    strcat(msg, b);

    return msg;
}

/*
* Handle player's input game choice
* If player says 'J', prompt for join code
* If player says 'C', initialize new game, and add game struct ptr to player
*/
void handle_choice(int client_fd, char *choice) {
    Player *player = clients[client_fd];
    
    if (strcmp(choice, "J") == 0) {
        write_to_client(client_fd, CODE);
        player->state = WAITING_CODE;
        return;
    } else if (strcmp(choice, "C") == 0) {
        player->state = WAITING_WORD;
        
        Game *game = init_game(player); 
        player->game = game;

        char code[5];
        sprintf(code, "%d", game->join_code);

        char *msg = generate_msg(CMD_CODE, code);
        write_to_client(client_fd, msg);
        return;
    }
}

/*
* Search for player with this fd and handle based on current player state.
*/
void handle_player(int client_fd) {
    Player *player = clients[client_fd];
    enum PlayerState state = player->state;
    
    while ((msg = extract_msg(player)) != NULL) {
        if (state == WAITING_NAME) {
            if (strlen(msg) >= sizeof(player->name) - 1) {
                write_to_client(player->fd, NAME); 
                return;
            }
            strncpy(player->name, msg, sizeof(player->name));

            // update player state and ask for player's game choice
            player->state = WAITING_GAME_CHOICE;
            write_to_client(player->fd, CHOICE);

        } else if (state == WAITING_GAME_CHOICE) {
            handle_choice(player->fd, msg);

        } else if (state == WAITING_CODE) {
            char *endptr;
            int code = (int)strtol(msg, &endptr, 10);
            
            // ERROR CHECKING: 
            if (endptr == msg || *endptr != '\0') {
                write_to_client(player->fd, CODE);
                free(msg); 
                return;
            } 

            Game *game = find_game_by_code(code); // TODO: add helper in game.c

            if (game != NULL && game->state == WAITING_FOR_PLAYER) {
                add_player(player, game); 
                player->state = WAITING_WORD;
                write_to_client(player->fd, WORD);
            } else {
                // Invalid code or game full, ask for code again
                write_to_client(player->fd, CODE);
            }

        } else if (state == WAITING_WORD) {
            if (valid_word(msg)) { 
                set_word(player, msg); 
                player->state = WAITING_GUESS;
                
                Game *game = player->game;
                Player *opponent = (game->player1 == player) ? game->player2 : game->player1;

                // Check if second player is here and already submitted their word
                if (opponent != NULL && opponent->state == WAITING_GUESS) {
                    game->state = IN_PROGRESS;
                    
                    char *signal1 = generate_msg(BOARD, game->player1->board);
                    write_to_client(game->player1->fd, signal1);
                    free(signal1); 
                    
                    char *signal2 = generate_msg(BOARD, game->player2->board);
                    write_to_client(game->player2->fd, signal2);
                    free(signal2);
                } else {
                    write_to_client(player->fd, STAT_WAIT);
                }
            } else {
                write_to_client(player->fd, WORD); 
            }

        } else if (state == WAITING_GUESS) {
            if (strlen(msg) != strlen(player->word)) {
                write_to_client(player->fd, LENGTH); 
                continue;
            }
            
            char *board = check_guess(player, msg);
            update_board(player, board);

            // check if word is same as player's word
            if (check_correct_word(player)) { 
                player->state = WAITING_SCORE; 
                write_to_client(player->fd, GUESSED_WORD);
                
                Game *game = player->game;
                Player *opponent = (game->player1 == player) ? game->player2 : game->player1;

                if (opponent->state == WAITING_SCORE) {
                    // Both players finished guessing
                    game->state = GAME_OVER;
                    finalize_scores(game); 
                    // Compare scores (guess counts) and send win/lose 
                    if (game->player1_score > game->player2_score) {
                        write_to_client(game->player1->fd, STAT_WIN);
                        write_to_client(game->player2->fd, STAT_LOSE);
                    } else if (game->player2_score < game->player1_score) {
                        write_to_client(game->player2->fd, STAT_WIN);
                        write_to_client(game->player1->fd, STAT_LOSE);
                    } else {
                        // TODO: handle ties - make STAT_TIE? 
                    }
                } else {
                    // Opponent is still guessing
                    write_to_client(player->fd, STAT_WAIT);
                }
                
            } else {
                // Incorrect guess, send updated board 
                char *signal = generate_msg(BOARD, player->board);
                write_to_client(player->fd, signal);
                free(signal);
            }
        }

        free(msg); 
    }
}


int main() {
    /* main flow:
        start server and wait
        once client joins, use select to get client input
        handle_client based on player and game state 
    */

    // random port
    struct sockaddr_in *self= init_server_addr(43465);
    int listenfd= set_up_server_socket(self, (MAX_SESSIONS) * 2);

    int numfd = listenfd;

    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(listenfd, &read_fds);

        // add all connected clients to read_fds 
        for (int fd = 0; fd <= numfd; fd++) {
            if (clients[fd] != NULL) {
                FD_SET(fd, &read_fds);        
            }
        }

        // select  
        if (select(numfd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("server: select");
            exit(1);
        }
        
        // check which ones are actually ready
        if (FD_ISSET(listenfd, &read_fds)) {
            int client_fd = accept_connection(listenfd); 
            clients[client_fd] = init_player(client_fd);
            write_to_client(client_fd, NAME);

            if (client_fd > numfd) {
                numfd = client_fd;
            }
        }

        // check the other clients:
        for (fd = 0; fd <= numfd; fd++) {
            if (clients[fd] != NULL && FD_ISSET(fd, &read_fds)) {
                read_client_msg(clients[fd]);
                handle_player(fd);
            }
        } 
    }


}
