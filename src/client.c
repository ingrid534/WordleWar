#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>    /* Internet domain header */
#include <arpa/inet.h>   /* inet_ntoa() - might only need on mac */ 
#include "_CLIENT_H_"
#include "PROTOCOL_H"

#define BUFSIZE 13 // 2 extra characters for sending end of line characters

// where clients (players) will be initialized and connect to server
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

void give_name(int soc) {
    // prompt player for their name
}

void give_game_choice(int soc) {
    // ask player whether they want to join existing game or create new one.
}

void give_name(int soc) {
    // prompt player for their name
}

void give_game_choice(int soc) {
    // ask player whether they want to join existing game or create new one.
}

/*
* Prompt user to input name and send name back to server
*/
void prompt_name(int soc) {
    // prompt user to input name
}

/*
* Give user choice of either joining existing game or creating new one
* // TODO: i wasn't sure why give_choice was returning a char before so i made it return void... I may have misunderstood so feel free to change it back.
*/
void give_choice(int soc){
     // Space for 3 characters; adding end of line characters later
    char option[3];
    
    //Get character from user
    fprintf(stdout, "Please enter C to create a new game and J to join an existing game: ");
    fgets(option, 2, stdin); // Space for null terminator and character
    while(strcmp(guess, "J")!=0 && strcmp(guess, "C")!=0 ){
        fprintf(stdout, "Invalid Selection ");
        fprintf(stdout, "Please enter C to create a new game and J to join an existing game: ");
        fgets(option, 2, stdin); // Space for null terminator and character
    }

    write_to_server(soc, option, 3);
    return;

}

/*
* Prompt user to input join code and send join code back to server.
* should only have basic checking (make sure user input a number), server will check if it is correct code
* and handle future calls to client
*/
void prompt_code(int soc) {
    // TODO: implement
}

/*
* Prompt user to input a word (for other player to guess) and send back to server.
*/
void prompt_word(int soc){
    char buf[BUFSIZE];
    
    //Get word from user
    fprintf(stdout, "Please enter a word for your opponent to guess:");
    fgets(buf, BUFSIZE, stdin);

    // Send chosen word to server; does not include null termination character
    if(write(soc, buf, strlen(buf))==-1){
        perror("write");
        exit(1);
    }

}

/*
* Prompt user to input letter guess, send guess back to server.
* Display here will include the formatted string sent from the server (wordle style)
* e.g. If "--A--e" is displayed, then we know "a" is the correct letter in the correct place, 
* "e" is a letter in the word but in the wrong place, and the other letters were incorrect.
*/
void prompt_guess(int soc){
    // Space for 3 characters; adding end of line characters later
    char guess[3];
    
    //Get character from user
    fprintf(stdout, "Please enter your guess:");
    fgets(guess, 2, stdin); // Space for null terminator and character

    write_to_server(soc, guess,3);

}

/*
* Tell user they guessed correct word.
* This function will be called only when user is the last in the game to guess their word,
* so no 'wait for the other player...' message is required.
*/
void give_correct_word(int soc) {
    // TODO: implement
}

/*
* Tell user they guessed correct word and must wait for the other player to finish.
*/
void give_wait(int soc) {
    // TODO: implement
}

/*
* Tell the user they won the game (plus score...?)
*/
void give_win(int soc) {
    // TODO: implement
}

/*
* Tell the user they lost the game (plus score...?)
*/
void give_lost(int soc) {
    // TODO: implement
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

char *validate_game_entries(int soc, char *line_read, char *msg, char *user_prompt){

     // Ensure server sent msg; otherwise error
    int match = strcmp(line_read, msg);

    // Free dynamically allocated memory; no use anymore
    free(line_read);

    // If the server sent msg, prompt user
    if (match == 0){
        fprintf(stdout, prompt);
        // Get user input
        char user_input[BUFSIZE];
        fgets(user_input, BUFSIZE, stdin);
        // If the user is supposed to input the code; we check if the input contains a number
        if (msg == CODE){
            char *end;
            long code = strtol(user_input, &end, 10);
            while(user_input == end){
                fprintf(stdout, "Invalid. No code found:");
                fprintf(stdout, prompt);
                fgets(user_input, BUFSIZE, stdin);
                code = strtol(user_input, &end, 10);
            }
            // At this point, the user entered a number; convert it back to text
            snprintf(user_input, BUFSIZE, "%ld", code);
        }

        // Write user input
        write_to_server(soc, user_input, BUFSIZE);
    }
    else{
        fprintf(stderr, "Unexpected message from server.");
        exit(1);
    }

    line_read = read_server_msg(soc);
    // If the user enters invalid parameter, server keeps prompting until they provide the right one
    while(strcmp(line, msg) == 0){
        // Free dynamically allocated memory
        free(line_read);

        fprintf("Invalid. Try again.");
        fprintf(stdout, prompt);

        // Get user input
        char user_input[BUFSIZE];
        fgets(user_input, BUFSIZE, stdin);

        // Write user input
        write_to_server(soc, user_input, BUFSIZE);

        line_read = read_server_msg(soc);

    }

    // At this point, user input was correct. Send last read line back
    return line_read;
}

// Check the exit message for other details
int check_exit_status(char *line_read){
    if (strstr(line_read, WIN) != NULL){
        free(line_read);
        fprintf(stdout, "You win!");
        return 0;
    }
    else if(strstr(line_read, LOSE) != NULL){
        free(line_read);
        fprintf(stdout, "You lose!");
        return 0;
    }
    else{ // Server told client to wait
        free(line_read);
        fprintf(stdout, "Wait for opponent.");
        return 1;

    }
}
//Communicate if guess was correct or not
void check_guess(char *line_read){
    if (strstr(line_read, CORRECT) != NULL){
        fprintf(stdout, "Correct guess.");
    }
    else if(strstr(line_read, INCORRECT) != NULL){
        fprintf(stdout, "Incorrect guess.");

    }
    else{ 
        fprintf(stdout, "Already guessed this character.");

    free(line_read);

}
char game_settings(int soc){
     // Space for 3 characters; adding end of line characters later
    char option[3];
    
    //Get character from user
    fprintf(stdout, "Please enter C to create a new game and J to join an existing game: ");
    fgets(option, 2, stdin); // Space for null terminator and character

    //Ensure they entered valid letter
    while(strcmp(guess, "J")!=0 && strcmp(guess, "C")!=0 ){
        fprintf(stdout, "Invalid Selection ");
        fprintf(stdout, "Please enter C to create a new game and J to join an existing game: ");
        fgets(option, 2, stdin); // Space for null terminator and character
    }

    write_to_server(soc, option,3);
    return option;

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

    // proposed loop:
        // client does read_server_msg
        // client checks if it's message it needs - ?? (e.g. "name" request for a client must come before "choice")
        // handle it with a helper (not straight inside main)

    // not sure what to have as loop condition
    while (1) {
        char *line_read = read_server_msg(server_socket); 

        if (strcmp(line_read, NAME) == 0) {
            prompt_name(server_socket); 
            free(line_read);
        } else if (strcmp(line_read, CHOICE) == 0) {
            give_choice(server_socket);
            free(line_read);
        } else if (strcmp(line_read, CODE) == 0) {
            prompt_code(server_socket);
            free(line_read);
        } else if (strcmp(line_read, WORD) == 0) {
            prompt_word(server_socket); 
            free(line_read);
        } else if (strcmp(line_read, GUESS) == 0) { // TODO: use strstr
            prompt_guess(server_socket); 
            free(line_read);
        } else if (strcmp(line_read, GUESSED_WORD) == 0) {
            give_correct_word(server_socket);
            free(line_read);
        } else if (strcmp(line_read, STAT_WAIT) == 0) {
            give_wait(server_socket); 
            free(line_read);
        } else if (strcmp(line_read, STAT_WIN) == 0) { // TODO: use strstr
            give_win(server_socket); 
            free(line_read);
            break;
        } else if (strcmp(line_read, STAT_LOST) == 0) { // TODO: use strstr
            give_lost(server_socket); 
            free(line_read);
            break;
        } else {
            fprintf(stderr, "Unexpected message from server.");
            free(line_read);
            exit(1);
        }
    }


    // MOVE STUFF BELOW HERE TO HELPERS DEFINED ABOVE ---------------------------

    if (strcmp(line_read, NAME) != 0){
        fprintf(stderr, "Unexpected message from server.");
        exit(1);

    }
    free(line_read); // Free dynamically allocated memory
    //Prompt user for name
    char name[MAX_BUF];
    fprintf(stdout, "Please enter your username:")
    fgets(buf, BUFSIZE, stdin);
    write_to_server(server_socket, buf, BUFSIZE);

    //Next we expect the server to ask if we want to join a game or create a game
    line_read = read_server_msg(server_socket);

    if (strcmp(line_read, GAME_OPTIONS) != 0){
        fprintf(stderr, "Unexpected message from server.");
        exit(1);

    }
    free(line_read); // Free dynamically allocated memory

    //Get user input for game options
    char option = game_settings(int server_socket);

    if (option == 'J'){
        // Expect the server to ask for code
        line_read = read_server_msg(server_socket);
        // Note validate game entries will free line_read
        char * next_line = validate_game_entries(server_socket, line_read, CODE, "Please enter the code:")

    }
    else{
        //Expect the server to return code
        line_read = read_server_msg(server_socket);

        //Print code
        fprintf(stdout, "Game code: %s", line_read)

        //Free line
        free(line_read);

        //Communicate to server that you are ready to play
        char server_msg[BUFSIZE];
        strcpy(server_msg, "Ready")
        write_to_server(server_socket, server_msg, BUFSIZE);
    }
    
    // At this point, we know the user has set up the game correctly. We expect the server to ask for a word
    char * next_line = validate_game_entries(server_socket, next_line, WORD, "Please enter the word for your opponent to guess:")

    //At this point, we know they have provided a valid word. We expect the server to have notified us that the game has begun, to send our first guess, and number of letters in our word.
    if (strstr(next_line, BEGIN) == NULL){
        fprintf(stderr, "Unexpected message from server.");
        exit(1);

    }
    // We expect the server to have provided number of letters for our word and number of guesses
    char *num_guesses;
    long length_word = strtol(next_line, &num_guesses, 10);
    fprintf("Your word has %ld characters.", length_word);
    long max_guesses = strtol(num_guesses, NULL, 10);
    fprintf("You have %ld guesses.", max_guesses);
    //Free dynamically allocated memory
    free(next_line);


    // Client sends first guess
    guess_letter(server_socket);
    line_read = read_server_msg(server_socket);

    // As long as the server has not said the game has ended, user keeps sending guesses
    while(strstr(line_read, EXIT) == NULL){
        check_guess(line_read); // Tell user status of their guess
        guess_letter(server_socket); // Tell user to input new guess
        line_read = read_server_msg(server_socket); // Read server response

    }
    // At this point, we know the server has said the game is done. But the remaining of the exit message, could contain wait, win or lose depending on whether the opponent is done
    if (check_exit_status(line_read) == 1){
        line_read = read_server_msg(server_socket);
        check_exit_status(line_read) == 1
    }


 
}