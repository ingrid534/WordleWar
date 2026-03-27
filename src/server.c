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
#include "_CLIENT_H"

#define MAX_SESSIONS 50;
#define BUFSIZE 11;

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
    if (write(client_fd, msg, strlen(msg)) == -1) {
        perror("server: write");
        exit(1);
    }
}

char *read_client_msg(int client_fd) {
    char *line = malloc(BUFSIZE);
    if (line == NULL) {
        perror("server: malloc");
        exit(1);
    }

    int num_bytes = read(client_fd, line, BUFSIZE - 1);
    if (num_bytes == -1) {
        perror("server: read");
        exit(1);
    }
    line[num_bytes] = '\0';
    
    // read until \r\n
    while(strstr(line, "\r\n") == NULL){
        int result = read(soc, &line[num_bytes], MAX_BUF - num_bytes);
    
        if(result == -1){
            perror("read");
            exit(1);
        }
        num_bytes += result;

        line[num_bytes] = '\0';

    }
    line[num_chars - 2] = '\0'; 
    return line;

}

// initialize new player struct with this fd
Player *create_player(int client_fd) {
    Player *player = malloc(sizeof(struct Player));
    clients[client_fd] = player;
    player->fd = client_fd;

    // update player state
    enum PlayerState state = WAITING_NAME;
    player->state = state; 

    // write to client signaling to ask for name
    write_to_client(player_fd, "name");

    return player;
}

/*
Handle player's input game choice
If player says 'join', prompt for join code
If player says 'create', initialize new game, and add game struct ptr to player
Assuming that client.c will return strictly "join" or "create"
*/
void handle_choice(int client_fd, char *choice) {
    // TODO: implement this based on client.c implementation
    if (strcmp(choice, "join") == 0) {
        
    }
}

/*
* Search for player with this fd and handle based on current player state.
*/
void handle_player(int client_fd) {
   Player *player = clients[client_fd];
   enum PlayerState state = player->state;

    if (state == WAITING_NAME) {
        char *name = read_client_msg(player->fd);
        strncpy(player->name, name, sizeof(player->name));

        // update game state and ask for player's game choice
        player->state = WAITING_GAME_CHOICE;
        write_to_client(player->fd, "choice");


    } else if (state == WAITING_GAME_CHOICE) {
        char *choice = read_client_msg(player->fd);
        handle_choice(player->fd, choice);

        // change state and send next prompt
        enum PlayerState new_state = WAITING_WORD;
        player->state = new_state; 
        write_to_client(player->fd, "guess");

    } else if (state == WAITING_WORD)
        give_word(player->fd);
        // assign this word to player 2 in their game ...
        // should player store game code for easier search?
}


int main() {
    /* main flow:
        start server and wait
        make array of sessions (of size MAX_SESSIONS)
        once client joins, use select to get client input (for name)
        initialize player struct with name and fd (return value of accept_connection)
        
    */

    // random port
    struct sockaddr_in *self= init_server_addr(43465);
    int listenfd= set_up_server_socket(self, (MAX_SESSIONS) * 2);

    Player *clients[FD_SETSIZE] = {NULL};
    int numfd = listenfd;

    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(listenfd, &read_fds);

        // add all connected clients to read_fds 
        for (int fd = 0; fd < numfd; fd++) {
            if (clients[fd] != NULL) {
                FD_SET(clients[fd]);        
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
            clients[client_fd] = create_player(client_fd);
            if (clients[client_fd] > numfd) {
                numfd = client_fd;
            }
        }

        // check the other clients:
        for (fd = 0; fd < numfd; fd++) {
            if (clients[fd] != NULL && FD_ISSET(fd, &read_fds)) {
                handle_player(fd);
            }
        } 
    }


}
