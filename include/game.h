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
    int       player1_score;
    int       player2_score;
    int       join_code;            // 4-digit code for player2 to join
    int player1_score;
    int player2_score;
    int join_code; // 4-digit code for player2 to join
    GameState state;
 
    // player1_word is the word player1 has to guess (chosen by player2), and vice versa
    char player1_word[MAX_WORD_LENGTH + 1];
    char player2_word[MAX_WORD_LENGTH + 1];
    int  player1_word_length;       // length of player1's target word
    int  player2_word_length;       // length of player2's target word
 
    int  player1_guesses;
    int player1_guesses;
    int  player2_guesses;
    bool player1_solved;
    bool player2_solved;
} Game;
 
Game *init_game(Player *player1);                          // create game, generate join code
void  add_player(Player *player, Game *game);              // add player2, advance state
int   set_word(Game *game, Player *player, const char *word); // player sets word for opponent; 0 on invalid
 
// score a guess against the player's target word
// returns malloc'd string: '-' = wrong, lowercase = right letter wrong place, UPPER = correct place
// also updates guess count and solved state
char *check_guess(Player *player, const char *guess);
 
bool  check_correct_word(const char *guess, const char *target, int length); // exact match check
bool  is_game_over(Game *game);     // true when both players are done
void  finalize_scores(Game *game);  // calculate scores, set state to GAME_OVER
void  end_game(Game *game);         // free game struct
 
#endif // GAME_H
 
