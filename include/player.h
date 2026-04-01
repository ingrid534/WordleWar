// define the player struct here, and any functions that will be used to manage the player (initializing, resetting, etc.)
#ifndef PLAYER_H
#define PLAYER_H
#include "protocol.h"

#define MAX_WORD_LENGTH 8

typedef struct game Game;

typedef enum {
    WAITING_NAME,
    WAITING_GAME_CHOICE,
    WAITING_CODE,
    WAITING_OPPONENT,
    WAITING_WORD,
    WAITING_OPPONENT_WORD,
    WAITING_GUESS,
    WAITING_SCORE
} PlayerState;

typedef struct player {
    int fd;
    Game *game;
    char name[BUFSIZE];
    char word[MAX_WORD_LENGTH+1];
    char board[MAX_WORD_LENGTH+1];
    PlayerState state;
    char buf[BUFSIZE];
    int inbuf;
} Player;

Player *init_player(int fd);
void update_word(Player *player, const char *word);
void update_board(Player *player, const char *board);
void remove_player(Player *player);

#endif
