#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>    /* Internet domain header */
#include <arpa/inet.h>   /* inet_ntoa() - might only need on mac */ 
#include "_CLIENT_H_"

#define BUFSIZE 11
#define BEGINS "Game starts. Provide word."
#define EXITS "Game ends"
#define LOSE "lost"
#define WIN "win"
#define WAIT "wait"
#define WORD_ERROR "invalid word"
#define GUESS_ERROR "already guessed"
#define CODE "code"



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

void give_word(int soc){
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

void guess_letter(int soc){
    char guess[2];
    
    //Get word from user
    fprintf(stdout, "Please enter your guess:");
    fgets(guess, 2, stdin);

    // Send chosen word to server; does not include null termination character
    if(write(soc, guess, strlen(guess))==-1){
        perror("write");
        exit(1);
    }
    

}

char * read_server_msg(int soc){
    char * line = malloc(BUFSIZE);
    //Check system call
    if(line == NULL){
        perror("malloc");
        exit(1);
    }
    int num_bytes = read(soc, line, BUFSIZE-1);
    // Check if read call failed
    if(read == -1){
        perror("read");
        exit(1);
    }
    // Make result a string
    line[num_bytes] = '\0';

    // Read data until \r\n which indicates the end of a single message
    while(strstr(line, "\r\n") == NULL){
        int result = read(soc, &line[num_bytes], MAX_BUF - num_bytes);
    
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
    int soc = socket(AF_INET, SOCK_STREAM, 0);
    if (soc == -1){
        perror("socket");
        exit(1);
    }

    // Connect with server
    int server_socket = int connect_to_server(soc, 55317, "teach.cs.toronto.edu");

    // Server should ask for code as the first message
    char *line = read_server_msg(server_socket);

    // Ensure server is asking for code; otherwise error
    int match = strcmp(line, CODE);
    // Free dynamically allocated memory
    free(line);

    // If the server asked for a code, ask user for the code
    if (match == 0){
        int code;
        fprintf(stdout, "Please enter the code to join:");
        scanf("%d", &code);

        // Write user input; convert to network byte order
        if(write(soc, htons(code), sizeof(int))==-1){
            perror("write");
            exit(1);
        }
    }
    else{
        fprintf(stderr, "Unexpected message from server.");
        exit(1);
    }

    line = read_server_msg(server_socket);
    // If the user enters the incorrect code, server keeps prompting until they provide the right one
    while(strcmp(line, CODE) == 0){
        // Free dynamically allocated memory
        free(line);

        fprintf("Incorrect Code. Try again.");
        int code;
        fprintf(stdout, "Please enter the code to join:");
        scanf("%d", &code);

        // Write user input; convert to network byte order
        if(write(soc, htons(code), sizeof(int))==-1){
            perror("write");
            exit(1);
        }
        line = read_server_msg(server_socket);

    }
    // At this point, the last message the server sent was not to re-enter the code
    // Validate that the last message stored in line is to start the game
    if(line == BEGIN){
        //Call function to get 
    }
    else{
        fprintf(stderr, "Unexpected message from server. Terminate");
        exit(1);
    }

)

    
    


    

}