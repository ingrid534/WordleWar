#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>    /* Internet domain header */
#include <arpa/inet.h>   /* inet_ntoa() - might only need on mac */ 
#include "_SERVER_H_"

#define MAX_SESSIONS 50;

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

Player *create_player(int client_fd) {
    // initialize new player struct with this fd
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

    // make global var for the 100 - also im doing this rn cuz idk what else to do
    Player *clients[100] = {NULL};
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

        // select ... 
        if (select(numfd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("server: select");
            exit(1);
        }
        
        // check which ones are actually ready
        if (FD_ISSET(listenfd, &read_fds)) {
            int client_fd = accept_connection(listenfd); 
            clients[client_fd] = create_player(client_fd);
            if (clients[client_fd] > numfd) {
                numfd = clients[client_fd];
            }
        }

        // check the other clients:
        //for (fd = 0; fd < numfd ....)
        // handle player(player_struct)
            // create this function
    }


}
