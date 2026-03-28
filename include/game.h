// game.h - game struct and functions called by server.c
#ifndef GAME_H
#define GAME_H
 
#include <stdbool.h>
#include "player.h"
#include "words.h"
 
#define MAX_GUESSES 20
 
typedef enum game_state {
    WAITING_FOR_PLAYER, // waiting for second player to join
    WAITING_FOR_WORDS,  // both players connected, need to submit words
    IN_PROGRESS,        // both words set, game is running
    GAME_OVER
} GameState;
 
typedef struct game {
    Player   *player1;
    Player   *player2;
    int       player1_count;
    int       player2_count;
    int       join_code;            // 4-digit code for player2 to join
    GameState state;
 
    // each player picks a word for the other to guess
    char player1_word[WORD_LENGTH + 1];
    char player2_word[WORD_LENGTH + 1];
 
    int  player1_guesses;
    int  player2_guesses;
    bool player1_solved;
    bool player2_solved;
} Game;
 
Game *init_game(Player *player1);                                               // create game, generate join code
void  add_player(Player *player, Game *game);                                   // add player2, advance state
int   set_word(Game *game, Player *player, const char *word);                   // set the word a player chose; returns 0 on invalid
bool  check_guess(Game *game, Player *player, const char *guess, int result[]); // score a guess (2=green,1=yellow,0=grey)
bool  is_game_over(Game *game);                                                 // true when both players are done
void  finalize_scores(Game *game);                                              // calculate scores, set state to GAME_OVER
void  end_game(Game *game);                                                     // free game struct
 
#endif // GAME_H
 
