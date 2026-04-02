// game.h - game struct and functions called by server.c
#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include "words.h"

typedef struct player Player;

#define MAX_GUESSES 20
#define MAX_GAMES 256

typedef enum game_state {
    WAITING_FOR_PLAYER,
    WAITING_FOR_WORDS,
    IN_PROGRESS,
    GAME_OVER
} GameState;

typedef struct game {
    Player *player1;
    Player *player2;
    int player1_score;
    int player2_score;
    int join_code;
    GameState state;

    // Words each player must guess.
    char player1_word[MAX_WORD_LENGTH + 1];
    char player2_word[MAX_WORD_LENGTH + 1];
    int player1_word_length;
    int player2_word_length;

    int player1_guesses;
    int player2_guesses;
    bool player1_solved;
    bool player2_solved;
} Game;

Game *init_game(Player *player1);
Game *find_game_by_code(int code);
void add_player(Player *player, Game *game);
int set_word(Player *player, const char *word);
char *check_guess(Player *player, const char *guess);
bool check_correct_word(Player *player, const char *guess);
bool is_game_over(Game *game);
void finalize_scores(Game *game);
void end_game(Game *game);

#endif // GAME_H
