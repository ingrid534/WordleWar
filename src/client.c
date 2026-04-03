#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>    /* Internet domain header */
#include <arpa/inet.h>   /* inet_ntoa() - might only need on mac */ 
#include "client.h"
#include "protocol.h"

static void read_user_input(char *buf, int size) {
    if (fgets(buf, size, stdin) == NULL) {
        fprintf(stderr, "client: input closed\n");
        exit(1);
    }
}


// Clients connect to server
int connect_to_server(int soc, int port, const char *hostname){
    //initialize server address    
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);  
    memset(&server.sin_zero, 0, 8);

    //Find IP address of hostname
    struct addrinfo *ai;
    // Call populates list
    getaddrinfo(hostname, NULL, NULL, &ai);
    
    // Want first element in the list
    server.sin_addr = ((struct sockaddr_in *)ai -> ai_addr)->sin_addr;
     
    // free the memory that was allocated by getaddrinfo for this list
    freeaddrinfo(ai);

    int con = connect(soc, (struct sockaddr *)&server, sizeof(struct sockaddr_in));

    //Check if connection succeeeded
    if(con == -1){
        perror("connection failed");
        exit(1);
    }
    return con;

}

int prompt_name(int soc) {
    //Prompt user for name
    char name[BUFSIZE];
    fprintf(stdout, "Please enter your username: ");
    read_user_input(name, BUFSIZE);
    return write_to_server(soc, name, BUFSIZE);
}

int give_game_choice(int soc) {
    // send exactly one-character command: C or J
    char option[3];
    char input[BUFSIZE];

    while (1) {
        fprintf(stdout, "Please enter C to create a new game or J to join an existing game:  ");
        read_user_input(input, BUFSIZE);

        int len = strlen(input);
        while (len > 0 && (input[len - 1] == '\n' || input[len - 1] == '\r')) {
            input[--len] = '\0';
        }

        if (len == 1 && (input[0] == 'C' || input[0] == 'c' || input[0] == 'J' || input[0] == 'j')) {
            option[0] = (input[0] == 'c') ? 'C' : (input[0] == 'j') ? 'J' : input[0];
            option[1] = '\0';
            break;
        }

        fprintf(stdout, "Invalid selection. Enter exactly one letter: C or J.\n");
    }

    return write_to_server(soc, option, 3);
}


/*
* Prompt user to input join code and send join code back to server.
* should only have basic checking (make sure user input a number), server will check if it is correct code
* and handle future calls to client
*/
int prompt_code(int soc) {
    char user_input[BUFSIZE];
    fprintf(stdout, "Please enter game code: ");
    read_user_input(user_input, BUFSIZE);

    char *end;
    long code = strtol(user_input, &end, 10);

    while(user_input == end){
        fprintf(stdout, "Try again. Code must be a number.\n");
        fprintf(stdout, "Please enter game code: ");
        read_user_input(user_input, BUFSIZE);
        code = strtol(user_input, &end, 10);
    }

    snprintf(user_input, BUFSIZE, "%ld", code);

     // Write user input
    return write_to_server(soc, user_input, BUFSIZE);

}

/*
* Prompt user to input a word (for other player to guess) and send back to server.
*/
int prompt_word(int soc){

    char user_input[BUFSIZE];
    fprintf(stdout, "Please enter a word for your opponent to guess: ");
    read_user_input(user_input, BUFSIZE);

    // Strip newline characters
    int len = strlen(user_input);
    while (len > 0 && (user_input[len - 1] == '\n' || user_input[len - 1] == '\r')) {
        user_input[--len] = '\0';
    }

    // Ensure word is of valid length
    while(strlen(user_input) > 8 || strlen(user_input) < 5){
        fprintf(stdout, "Try again. Proposed word must be in between 5 to 8 characters. ");
        fprintf(stdout, "Please enter a word for your opponent to guess: ");
        read_user_input(user_input, BUFSIZE);

        // Strip newline characters
        len = strlen(user_input);
        while (len > 0 && (user_input[len - 1] == '\n' || user_input[len - 1] == '\r')) {
            user_input[--len] = '\0';
        }
    }

    // Write user input
    return write_to_server(soc, user_input, BUFSIZE);
}

/*
* Prompt user to input letter guess, send guess back to server.
* Display here will include the formatted string sent from the server (wordle style)
* e.g. If "--A--e" is displayed, then we know "a" is the correct letter in the correct place, 
* "e" is a letter in the word but in the wrong place, and the other letters were incorrect.
*/
int prompt_guess(int soc, const char *server_msg){
    // Formatted string contains board and num guesses left separated by & - double check this
    // i.e. Board: --A--e & Number of Guesses Left: 10
    char msg_copy[BUFSIZE];
    strncpy(msg_copy, server_msg, BUFSIZE - 1);
    msg_copy[BUFSIZE - 1] = '\0';

    char *num_guess_left = strstr(msg_copy, "&");
    if (num_guess_left != NULL) {
        *num_guess_left = '\0';
        printf("%s", num_guess_left + 1);
    }
    printf("%s", msg_copy);
    char guess[BUFSIZE];
    
    //Get guess from user
    fprintf(stdout, "\nPlease enter your guess. Capital letters mean letter is in correct spot. Lowercase letters mean letter is in the wrong spot: \n");
    fprintf(stdout, "------------------------\n");
    read_user_input(guess, BUFSIZE);

    return write_to_server(soc, guess,BUFSIZE);

}

/*
* Tell user they guessed correct word.
* This function will be called only when user is the last in the game to guess their word,
* so no 'wait for the other player...' message is required.
*/
void give_correct_word(int soc, const char *server_msg) {
    (void)soc;
    (void)server_msg;
    printf("You guessed the word!\n");
}

/*
* Tell user they guessed correct word and must wait for the other player to finish.
*/
void give_wait(int soc) {
    (void)soc;
    printf("Please wait for the other player to finish.\n");
}

/*
* Tell the user they won the game 
*/
void give_win(int soc, const char * server_msg) {
    (void)soc;
    char *score = strstr(server_msg, ":");
    if (score != NULL) {
        printf("You win! You guessed the word in %s guess(es)!.\n", score + 1);
    } else {
        printf("You win!\n");
    }

}

/*
* Tell the user they lost the game 
*/
void give_lost(int soc, const char * server_msg) {
    (void)soc;
    char *score = strstr(server_msg, ":");
    if (score != NULL) {
        printf("You lost! You guessed the word in %s guess(es).\n", score + 1);
    } else {
        printf("You lost!\n");
    }
}

/*
* Tell both users they tied
*/
void give_tie(int soc, const char *server_msg) {
    (void)soc;
    char *score = strstr(server_msg, ":");
    if (score != NULL) {
        printf("You tied! You both took %s guess(es).\n", score + 1);
    } else {
        printf("You tied!\n");
    }
}

int write_to_server(int soc, char *msg, int msg_buffer_size){
    int len = (int)strlen(msg);

    // Strip any existing line ending so we always send exactly one CRLF.
    while (len > 0 && (msg[len - 1] == '\n' || msg[len - 1] == '\r')) {
        len--;
    }

    if (len > msg_buffer_size - 2) {
        len = msg_buffer_size - 2;
    }

    msg[len] = '\r';
    msg[len + 1] = '\n';

    int length_to_send = len + 2;

    int inbuf = 0;
    int room = length_to_send;
    char *after = msg;
    ssize_t nbytes;

    while ((nbytes = write(soc, after, room)) > 0) {
        inbuf += (int)nbytes;
        room -= (int)nbytes;
        after = &msg[inbuf];
    }

    if (nbytes == -1 || room > 0) {
        fprintf(stderr, "Server disconnected.\n");
        close(soc);
        return -1;
    }
    return 0;

}

/*
* Notify player that their opponent disconnected and take them back to choice options.
*/
int give_disconnect(int soc) {
    printf("Your opponent disconnected :(\n");
    return give_game_choice(soc);
}

static int find_network_newline(const char *buf, int n) {
    for (int i = 0; i < n - 1; i++) {
        if (buf[i] == '\r' && buf[i + 1] == '\n') {
            return i;
        }
    }
    return -1;
}

char *read_server_msg(int soc){
    static char buf[BUFSIZE];
    static int inbuf = 0;

    while (1) {
        int where = find_network_newline(buf, inbuf);
        if (where >= 0) {
            char *line = malloc((size_t)where + 1);
            if (line == NULL) {
                perror("malloc");
                exit(1);
            }

            memcpy(line, buf, (size_t)where);
            line[where] = '\0';

            int consumed = where + 2;
            inbuf -= consumed;
            if (inbuf > 0) {
                memmove(buf, buf + consumed, (size_t)inbuf);
            }

            return line;
        }

        if (inbuf == BUFSIZE) {
            fprintf(stderr, "client: message too long\n");
            exit(1);
        }

        int nread = read(soc, buf + inbuf, (size_t)(BUFSIZE - inbuf));
        if (nread == -1) {
            fprintf(stderr, "Server disconnected.\n");
            close(soc);
            return NULL;
        }
        if (nread == 0) {
            return NULL;
        }

        inbuf += nread;
    }
}


int main(){
    // Create socket and exit on failure
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1){
        perror("socket");
        exit(1);
    }

    // ignore closed socket signal (let write handle the fail)
    signal(SIGPIPE, SIG_IGN);

    // Connect with server; only returns if connection successful
    connect_to_server(server_socket, PORT, "localhost");

        // client reads_server_msg
        // client checks which prompt
        // handle it with a helper 

    // Loop is infinite; eventually exits once the server prompts game has ended
    while (1) {
        char *line_read = read_server_msg(server_socket);
        if (line_read == NULL) {
            fprintf(stderr, "Server closed the connection.\n");
            break;
        }

        int write_status = 0;

        if (strcmp(line_read, NAME) == 0) {
            write_status = prompt_name(server_socket); 
            free(line_read);
        } else if (strcmp(line_read, CHOICE) == 0) {
            write_status = give_game_choice(server_socket);
            free(line_read);
        } else if (strcmp(line_read, GAME_FULL) == 0) {
            printf("Game capacity full. Wait or join a game.\n");
            write_status = give_game_choice(server_socket);
            free(line_read);
        } else if (strcmp(line_read, CODE) == 0) {
            write_status = prompt_code(server_socket);
            free(line_read);
        } else if(strcmp(line_read, INVALID_CODE) == 0){
            printf("Session does not exist.");
            write_status = prompt_code(server_socket);
            free(line_read);
        } else if (strcmp(line_read, WORD) == 0) {
            write_status = prompt_word(server_socket); 
            free(line_read);
        } else if(strcmp(line_read, INVALID_WORD) == 0){
            printf("Word does not exist in game dictionary.");
            write_status = prompt_word(server_socket); 
            free(line_read);
        } else if (strcmp(line_read, WAIT_OPPONENT_WORD) == 0) {
            printf("Word submitted. Waiting for your opponent to submit their word.\n");
            free(line_read);
        } else if(strstr(line_read, CMD_CODE) != NULL){
            printf("%s", line_read);
            printf("\n");
            free(line_read);
            // After creating a game, server expects this client to submit a word next.
            write_status = prompt_word(server_socket);

        } else if (strstr(line_read, BOARD) != NULL) { 
            write_status = prompt_guess(server_socket, line_read); 
            free(line_read);
        } else if (strcmp(line_read, LENGTH) == 0) {
            printf("Invalid guess length. Please use the same number of letters as shown on the board.\n");
            write_status = prompt_guess(server_socket, "");
            free(line_read);
        } else if (strcmp(line_read, GUESSED_WORD) == 0) {
            give_correct_word(server_socket, line_read); // need to preserve back and forth
            free(line_read);
        } else if (strcmp(line_read, OUT_OF_GUESSES) == 0) {
            printf("You are out of guesses. Waiting for the other player to finish.\n");
            free(line_read);
        } else if (strcmp(line_read, STAT_WAIT) == 0) { // need to preserve back and forth
            give_wait(server_socket); 
            free(line_read);
        } else if (strstr(line_read, STAT_WIN) != NULL) { 
            give_win(server_socket, line_read); 
            free(line_read);
            break;
        } else if (strstr(line_read, STAT_LOST) != NULL) { 
            give_lost(server_socket, line_read); 
            free(line_read);
            break;
        } else if (strstr(line_read, STAT_TIE) != NULL) { 
            give_tie(server_socket, line_read);
            free(line_read);
            break;
        } else if (strcmp(line_read, PLAYER_DISCONNECT) == 0) {
            write_status = give_disconnect(server_socket);
            free(line_read);
        } else {
            fprintf(stderr, "Unexpected message from server.");
            free(line_read);
            exit(1);
        }

        if (write_status == -1) {
            break;
        }
    }
    // check if socket was already closed in a read call before closing
    if (server_socket >= 0) {
        close(server_socket);
    }
 
}
