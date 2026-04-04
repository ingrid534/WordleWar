#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>    /* Internet domain header */
#include <arpa/inet.h>   /* inet_ntoa() - might only need on mac */ 

#include "server.h"
#include "player.h"
#include "game.h"
#include "protocol.h"
#include "words.h"

#define MAX_SESSIONS 50 

Player *clients[FD_SETSIZE] = {NULL};

/*
 * Initialize a server address associated with the given port.
 */
struct sockaddr_in *init_server_addr(int port) {
    struct sockaddr_in *addr = malloc(sizeof(struct sockaddr_in));
    if (addr == NULL) {
        perror("server: malloc");
        exit(1);
    }

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
    int status = setsockopt(listen_soc, SOL_SOCKET, SO_REUSEADDR,
                            (const char *) &on, sizeof(on));
    if (status < 0) {
        perror("setsockopt");
        exit(1);
    }

    // bind socket to addr
    if (bind(listen_soc, (struct sockaddr *) self, sizeof(struct sockaddr_in)) == -1) {
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
    socklen_t client_len = sizeof(struct sockaddr_in);

    // accept connection
    int client_soc = accept(listenfd, (struct sockaddr *) &client_addr, &client_len);
    if (client_soc < 0) {
        perror("server: accept");
        exit(1);
    }

    return client_soc;
}

void write_to_client(int client_fd, const char *msg) {
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s\r\n", msg);

    int inbuf = 0;
    int room = (int)strlen(buf);
    char *after = buf;
    ssize_t nbytes;

    while ((nbytes = write(client_fd, after, (size_t)room)) > 0) {
        inbuf += (int)nbytes;
        room -= (int)nbytes;
        after = &buf[inbuf];
    }

    if (nbytes == -1 || room > 0) {
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
    
    if (nbytes <= 0) {
        if (nbytes == 0) {
            printf("Client %d disconnected.\n", player->fd);
        } else {
            perror("server: read");
            printf("Treating read error from client %d as disconnect.\n", player->fd);
        }

        // if client was in a game, handle other player
        Game *game = player->game;
        if (game != NULL) {
            Player *opponent = (game->player1 == player) ? game->player2 : game->player1;
            if (opponent != NULL) {
                opponent->game = NULL;
                if (game->state != GAME_OVER) {
                    opponent->state = WAITING_GAME_CHOICE;
                    write_to_client(opponent->fd, PLAYER_DISCONNECT);
                }
            }
            end_game(game);
        }

        int fd = player->fd;
        close(fd);
        remove_player(player);
        clients[fd] = NULL;
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
        if (msg == NULL) {
            perror("server: malloc");
            exit(1);
        }
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
char *generate_msg(const char *a, const char *b) {
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

void write_status_with_score(int client_fd, const char *status_prefix, int score) {
    char score_buf[12];
    snprintf(score_buf, sizeof(score_buf), "%d", score);

    char *msg = generate_msg(status_prefix, score_buf);
    if (msg == NULL) {
        return;
    }

    write_to_client(client_fd, msg);
    free(msg);
}

void write_board_with_guesses_left(Player *player) {
    if (player == NULL || player->game == NULL) {
        return;
    }

    Game *game = player->game;
    int guesses_used = (game->player1 == player) ? game->player1_guesses : game->player2_guesses;
    int guesses_left = MAX_GUESSES - guesses_used;

    char board_msg[BUFSIZE];
    snprintf(board_msg, sizeof(board_msg), "%s:%d", player->board, guesses_left);

    char *signal = generate_msg(BOARD, board_msg);
    if (signal == NULL) {
        return;
    }

    write_to_client(player->fd, signal);
    free(signal);
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
        Game *game = init_game(player); 
        if (game == NULL) {
            player->state = WAITING_GAME_CHOICE;
            write_to_client(client_fd, GAME_FULL);
            return;
        }

        player->state = WAITING_WORD;
        player->game = game;

        char code[5];
        sprintf(code, "%d", game->join_code);

        char *msg = generate_msg(CMD_CODE, code);
        write_to_client(client_fd, msg);
        free(msg);
        return;
    }

    write_to_client(client_fd, CHOICE);
}

/*
* Search for player with this fd and handle based on current player state.
*/
void handle_player(int client_fd) {
    Player *player = clients[client_fd];
    char *msg;
    
    while ((msg = extract_msg(player)) != NULL) {
        PlayerState state = player->state;

        if (state == WAITING_NAME) {
            if (strlen(msg) >= sizeof(player->name) - 1) {
                write_to_client(player->fd, NAME); 
                free(msg);
                return;
            }
            strncpy(player->name, msg, sizeof(player->name));
            player->name[sizeof(player->name) - 1] = '\0';

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
                continue;
            } 

            Game *game = find_game_by_code(code); 

            if (game != NULL && game->state == WAITING_FOR_PLAYER) {
                add_player(player, game); 
                player->state = WAITING_WORD;
                write_to_client(player->fd, WORD);
            } else {
                // Invalid code or game full, ask for code again
                write_to_client(player->fd, INVALID_CODE);
                player->state = WAITING_GAME_CHOICE;
            }

        } else if (state == WAITING_WORD) {
            if (is_valid_word(msg) && set_word(player, msg)) {
                Game *game = player->game;
                Player *opponent = (game->player1 == player) ? game->player2 : game->player1;

                // Guessing starts only after both players have submitted words.
                if (opponent != NULL && opponent->state == WAITING_OPPONENT_WORD) {
                    player->state = WAITING_GUESS;
                    opponent->state = WAITING_GUESS;
                    game->state = IN_PROGRESS;

                    write_board_with_guesses_left(game->player1);
                    write_board_with_guesses_left(game->player2);
                } else {
                    player->state = WAITING_OPPONENT_WORD;
                    write_to_client(player->fd, WAIT_OPPONENT_WORD);
                }
            } else {
                write_to_client(player->fd, WORD); 
            }

        } else if (state == WAITING_OPPONENT_WORD) {
            write_to_client(player->fd, WAIT_OPPONENT_WORD);

        } else if (state == WAITING_GUESS) {
            if (strlen(msg) != strlen(player->word)) {
                write_to_client(player->fd, LENGTH); 
                continue;
            }

            Game *game = player->game;
            
            char *board = check_guess(player, msg);
            if (board == NULL) {
                write_board_with_guesses_left(player);
                continue;
            }
            update_board(player, board);
            free(board);

            bool solved = check_correct_word(player, msg);
            int guesses_used = (game->player1 == player) ? game->player1_guesses : game->player2_guesses;
            bool out_of_guesses = (guesses_used >= MAX_GUESSES);

            if (solved || out_of_guesses) {
                if (solved) {
                    write_to_client(player->fd, GUESSED_WORD);
                    player->state = WAITING_SCORE_GUESSED;
                } else {
                    write_to_client(player->fd, OUT_OF_GUESSES);
                    player->state = WAITING_SCORE_FAILED;
                }

                Player *opponent = (game->player1 == player) ? game->player2 : game->player1;

                if (opponent != NULL && (opponent->state == WAITING_SCORE_GUESSED || opponent->state == WAITING_SCORE_FAILED)) {
                    // Both players finished guessing
                    game->state = GAME_OVER;
                    finalize_scores(game); 
                    // Compare scores (guess counts) and send win/lose (stat_failed if player failed to guess)
                    if (game->player1_score > game->player2_score) {
                        write_status_with_score(game->player1->fd, STAT_WIN, game->player1_guesses);
                        if (game->player2->state== WAITING_SCORE_GUESSED) {
                            write_status_with_score(game->player2->fd, STAT_LOST, game->player2_guesses);
                        } else {
                            write_to_client(game->player2->fd, STAT_FAILED);
                        }

                    } else if (game->player2_score > game->player1_score) {
                        write_status_with_score(game->player2->fd, STAT_WIN, game->player2_guesses);
                        if (game->player1->state == WAITING_SCORE_GUESSED) {
                            write_status_with_score(game->player1->fd, STAT_LOST, game->player1_guesses);
                        } else {
                            write_to_client(game->player1->fd, STAT_FAILED);
                        }
                        
                    } else if (game->player1->state == WAITING_SCORE_GUESSED 
                            && game->player2->state == WAITING_SCORE_FAILED) {
                        write_status_with_score(game->player1->fd, STAT_WIN, game->player1_guesses);
                        write_to_client(game->player2->fd, STAT_FAILED);

                    } else if (game->player1->state == WAITING_SCORE_FAILED
                            && game->player2->state == WAITING_SCORE_GUESSED) {
                        write_to_client(game->player1->fd, STAT_FAILED);
                        write_status_with_score(game->player2->fd, STAT_WIN, game->player2_guesses);

                    } else if (game->player1->state == WAITING_SCORE_GUESSED) {
                        write_status_with_score(game->player1->fd, STAT_TIE, game->player1_guesses);
                        write_status_with_score(game->player2->fd, STAT_TIE, game->player1_guesses);
                    } else {
                        write_to_client(game->player1->fd, STAT_FAILED);
                        write_to_client(game->player2->fd, STAT_FAILED);
                    }

                    if (game->player1 != NULL) {
                        game->player1->game = NULL;
                    }
                    if (game->player2 != NULL) {
                        game->player2->game = NULL;
                    }
                    end_game(game);
                } else {
                    // Opponent is still guessing
                    write_to_client(player->fd, STAT_WAIT);
                }
                
            } else {
                // Incorrect guess, send updated board 
                write_board_with_guesses_left(player);
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

    // Load dictionary once at server startup so is_valid_word has data.
    load_word_bank("word.txt");
    if (word_count == 0) {
        fprintf(stderr, "server: word bank is empty\n");
        exit(1);
    }

    signal(SIGPIPE, SIG_IGN);

    struct sockaddr_in *self= init_server_addr(PORT);
    int listenfd= set_up_server_socket(self, MAX_SESSIONS);

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
            Player *new_player = init_player(client_fd);
            if (new_player == NULL) {
                fprintf(stderr, "server: failed to allocate player for fd %d\n", client_fd);
                close(client_fd);
            } else {
                clients[client_fd] = new_player;
                write_to_client(client_fd, NAME);

                if (client_fd > numfd) {
                    numfd = client_fd;
                }
            }
        }

        // check the other clients:
        for (int fd = 0; fd <= numfd; fd++) {
            if (clients[fd] != NULL && FD_ISSET(fd, &read_fds)) {
                read_client_msg(clients[fd]);
                if (clients[fd] != NULL) {
                    handle_player(fd);
                }
            }
        } 
    }


}
