// define the player struct here, and any functions that will be used to manage the player (initializing, resetting, etc.)
#ifndef PLAYER_H
#define PLAYER_H
#define MAX_NAME_LENGTH 20

typedef enum {
    WAITING_NAME,
    WAITING_GAME_CHOICE,
    WAITING_WORD,
    WAITING_GUESS,
    WAITING_PLAYER
} PlayerState;

typedef struct player {
    int fd;
    char name[MAX_NAME_LENGTH];
    char word[6]; // word this player has to guess
    bool in_game;
    PlayerState state;
} Player;

// initialize player
Player *init_player(int fd, char *name);

// update player status to in game 
void join_game(Player *player);

// update player's word to guess
void set_player_word(Player *player, char *word);

// reset player status to not in game (they exited the game)
void exit_game(Player *player); 

// free player struct memory (when player completely exits the server)
void remove_player(Player *player);

#endif
