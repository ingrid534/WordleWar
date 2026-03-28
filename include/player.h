// define the player struct here, and any functions that will be used to manage the player (initializing, resetting, etc.)
#ifndef PLAYER_H
#define PLAYER_H
#define MAX_NAME_LENGTH 20

typedef enum {
    WAITING_NAME,
    WAITING_GAME_CHOICE,
    WAITING_CODE,
    WAITING_WORD,
    WAITING_GUESS,
    WAITING_SCORE
} PlayerState;

typedef struct player {
    int fd;
    Game *game;
    char name[MAX_NAME_LENGTH];
    PlayerState state;
} Player;

Player *init_player(int fd, char *name);
void join_game(Player *player);

// update player's word they have to guess
void exit_game(Player *player); 
void remove_player(Player *player);

#endif
