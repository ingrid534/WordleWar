// to define game struct, and any functions that will be used to manage the game (initializing, guess checking, etc.)
#ifndef GAME_H
#define GAME_H 
#include "player.h"

/*
Current idea: each game (between two players) can have multiple rounds. Players can choose to quit at any time, in which case the struct will be deleted. If they choose to play again, the game will reset back to WAITING_FOR_WORDS game state.

Right now, we can stick to having each game just be one round, so player score will be the number of guesses they had left (20 - num_guesses) - that way player with smallest number of guesses will have higher score.
*/
typedef enum game_state {
    WAITING_FOR_PLAYER,
    WAITING_FOR_WORDS,
    IN_PROGRESS,
    GAME_OVER 
} GameState;

typedef struct game {
    Player *player1;
    Player *player2;
    int player1_count;
    int player2_count;
    char *player1_word;
    char *player2_word;
    int join_code;
    GameState state;
} Game;

// initialize game
Game *init_game(Player *player1);

// add player to this game
void add_player(Player *player, Game *game);

// check guess
bool check_guess(Game *game, Player *player, char *guess);

// reset game - change game status back for WAITING_FOR_WORDS - this is later if we decide to add this

// end game - free the memory for this struct - player structs should still exist after this.
void end_game(Game *game);

#endif