#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>    /* Internet domain header */
#include <arpa/inet.h>   /* inet_ntoa() - might only need on mac */ 
#include "_CLIENT_H_"
#include "PROTOCOL_H"


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
    getaddrinfo(hostname, NULL, NULL, &ai)

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

void prompt_name(int soc) {
    //Prompt user for name
    char name[BUFSIZE];
    fprintf(stdout, "Please enter your username:")
    fgets(buf, BUFSIZE, stdin);
    write_to_server(server_socket, buf, BUFSIZE);
}

void give_game_choice(int soc) {
    // Space for 3 characters; adding end of line characters after
    char option[3];
    
    //Get character from user
    fprintf(stdout, "Please enter C to create a new game and J to join an existing game: ");
    fgets(option, 2, stdin); // Space for null terminator and character

    //Ensure they entered valid letter
    while(strcmp(guess, "J")!=0 && strcmp(guess, "C")!=0 ){
        fprintf(stdout, "Invalid Selection ");
        fprintf(stdout, "Please enter C to create a new game or J to join an existing game: ");
        fgets(option, 2, stdin); // Space for null terminator and character
    }

    write_to_server(soc, option,3);
}


/*
* Prompt user to input join code and send join code back to server.
* should only have basic checking (make sure user input a number), server will check if it is correct code
* and handle future calls to client
*/
void prompt_code(int soc) {
    char user_input[BUFSIZE];
    fprintf("Please enter game code:");
    fgets(user_input, BUFSIZE, stdin);

    char *end;
    long code = strtol(user_input, &end, 10);

    while(user_input == end){
        fprintf(stdout, "Try again. Code must be a number. ");
        fprintf("Please enter game code:");
        fgets(user_input, BUFSIZE, stdin);
        code = strtol(user_input, &end, 10);
    }

    snprintf(user_input, BUFSIZE, "%ld", code);

     // Write user input
    write_to_server(soc, user_input, BUFSIZE);

}

/*
* Prompt user to input a word (for other player to guess) and send back to server.
*/
void prompt_word(int soc){

    char user_input[BUFSIZE];
    fprintf("Please enter a word for your opponent to guess:");
    fgets(user_input, BUFSIZE, stdin);


    // Ensure word is of valid length
    while(strlen(user_input) > 8 || strlen(user_input < 5)){
        fprintf(stdout, "Try again. Proposed word must be in between 5 to 8 characters. ");
        fprintf("Please enter a word for your opponent to guess:");
        fgets(user_input, BUFSIZE, stdin);
    }

    // Write user input
    write_to_server(soc, user_input, BUFSIZE);
}

/*
* Prompt user to input letter guess, send guess back to server.
* Display here will include the formatted string sent from the server (wordle style)
* e.g. If "--A--e" is displayed, then we know "a" is the correct letter in the correct place, 
* "e" is a letter in the word but in the wrong place, and the other letters were incorrect.
*/
void prompt_guess(int soc, char* server_msg){
    // Formatted string contains board and num guesses left separated by & - double check this
    // i.e. Board: --A--e & Number of Guesses Left: 10
    char *num_guess_left = strstr(server_msg, "&");
    *num_guess_left = '\0';

    printf("%s", num_guess_left + 1);
    printf("%s", server_msg)

    char guess[BUFSIZE];
    
    //Get guess from user
    fprintf(stdout, "Please enter your guess. Capital letters mean letter is in correct spot. Lowercase letters mean letter is in the wrong spot:");
    fgets(guess, BUFSIZE, stdin);

    write_to_server(soc, guess,BUFSIZE);

}

/*
* Tell user they guessed correct word.
* This function will be called only when user is the last in the game to guess their word,
* so no 'wait for the other player...' message is required.
*/
void give_correct_word(int soc, char* server_msg) {
    printf(server_msg); // Depends on how server sends the prompt
    // Must send a message back to preserve back and forth
    char response[BUFSIZE] = "Received";
    write_to_server(server_socket, response,BUFSIZE);
}

/*
* Tell user they guessed correct word and must wait for the other player to finish.
*/
void give_wait(int soc) {
    printf("You guessed the word! Please wait for the other player to finish.");
    // Must send a message back to preserve back and forth
    char response[BUFSIZE] = "Received";
    write_to_server(server_socket, response,BUFSIZE);
}

/*
* Tell the user they won the game (plus score...?)
*/
void give_win(int soc, char * server_msg) {
    char *score = strstr(server_msg, ":");
    printf("You win! You took %s guesses.", score + 1);
    exit(1); // SHOULD WE EXIT; TO DO

}

/*
* Tell the user they lost the game (plus score...?)
*/
void give_lost(int soc, char * server_msg) {
    char *score = strstr(server_msg, ":");
    printf("You lost! You took %s guesses.", score + 1);
    exit(1); // SHOULD WE EXIT; TO DO
}

void write_to_server(int soc, char *msg, int msg_buffer_size){
    // Appending end of line characters
    int length_to_send;
    if(strlen(msg) >= msg_buffer_size - 1){
        //if msg fills up most of the buffer, we must delete the last 2 characters
        msg[msg_buffer_size -1 ] = "\n";
        msg[msg_buffer_size -2 ] = "\r";
        length_to_send = msg_buffer_size;

    }
    else {
        int len = strlen(msg);
        msg[len] = "\n"; //Re
        msg[len + 1 ] = "\r";
        length_to_send = len + 1;

    }

    // Send to server; does not include null termination character
    if(write(soc, msg, length_to_send)==-1){
        perror("write");
        exit(1);
    }

}

// This works because we expect the server to send 1 message at a time
char *read_server_msg(int soc){
    char *line = malloc(BUFSIZE);
    //Check system call
    if(line == NULL){
        perror("malloc");
        exit(1);
    }
    int num_bytes = read(soc, line, BUFSIZE-1);
    // Check if read call failed
    if(num_bytes == -1){
        perror("read");
        exit(1);
    }
    // Make result a string
    line[num_bytes] = '\0';

    // Read data until \r\n which indicates the end of a single message
    while(strstr(line, "\r\n") == NULL){
        int result = read(soc, &line[num_bytes], MAX_BUF - num_bytes);
        // TODO: what is max_buf?
    
        if(result == -1){
            perror("read");
            exit(1);
        }
        num_bytes += result;

        line[num_bytes] = '\0';

    }
    line[num_chars - 2] = '\0'; // replace \r\n with null terminator
    return line;

}


int main(){
    // Create socket and exit on failure
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1){
        perror("socket");
        exit(1);
    }

    // Connect with server; only returns if connection successful
    connect_to_server(server_socket, 43465, "teach.cs.toronto.edu");

        // client reads_server_msg
        // client checks which prompt
        // handle it with a helper 

    // Loop is infinite; eventually exits once the server prompts game has ended
    while (1) {
        char *line_read = read_server_msg(server_socket); 

        if (strcmp(line_read, NAME) == 0) {
            prompt_name(server_socket); 
            free(line_read);
        } else if (strcmp(line_read, CHOICE) == 0) {
            give_game_choice(server_socket);
            free(line_read);
        } else if (strcmp(line_read, CODE) == 0) {
            prompt_code(server_socket);
            free(line_read);
        } else if(strcmp(line_read, INVALID_CODE) == 0){
            printf("Session does not exist.");
            prompt_code(server_socket);
            free(line_read);
        } else if (strcmp(line_read, WORD) == 0) {
            prompt_word(server_socket); 
            free(line_read);
        } else if(strcmp(line_read, INVALID_WORD) == 0){
            printf("Word does not exist in game dictionary.");
            prompt_word(server_socket); 
            free(line_read);
        } else if(strstr(line_read, GAME_CODE) != NULL){
            printf(line_read);
            free(line_read);
            // Must send a message back to preserve back and forth
            char response[BUFSIZE] = "Received";
            write_to_server(server_socket, response,BUFSIZE);

        } else if (strstr(line_read, BOARD) != NULL) { // TODO: use strstr
            prompt_guess(server_socket); 
            free(line_read);
        } else if (strcmp(line_read, GUESSED_WORD) == 0) {
            give_correct_word(server_socket); // need to preserve back and forth
            free(line_read);
        } else if (strcmp(line_read, STAT_WAIT) == 0) { // need to preserve back and forth
            give_wait(server_socket); 
            free(line_read);
        } else if (strstr(line_read, STAT_WIN) != NULL) { 
            give_win(server_socket); 
            free(line_read);
            break;
        } else if (strstr(line_read, STAT_LOST) != NULL) { 
            give_lost(server_socket); 
            free(line_read);
            break;
        } else {
            fprintf(stderr, "Unexpected message from server.");
            free(line_read);
            exit(1);
        }
    }
 
}
