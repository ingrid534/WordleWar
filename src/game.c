// game struct, will manage game logic, guess validation, etc.
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "game.h"
 
// lowercase src into dst (assumes src is WORD_LENGTH alpha chars)
static void lowercase_word(char dst[WORD_LENGTH + 1], const char *src) {
    for (int i = 0; i < WORD_LENGTH; i++)
        dst[i] = (char)tolower((unsigned char)src[i]);
    dst[WORD_LENGTH] = '\0';
}
 
Game *init_game(Player *player1) {
    Game *game = malloc(sizeof(Game));
    if (!game) { perror("init_game: malloc"); exit(1); }
 
    game->player1         = player1;
    game->player2         = NULL;
    game->player1_score   = 0;
    game->player2_score   = 0;
    game->join_code       = rand() % 9000 + 1000;
    game->state           = WAITING_FOR_PLAYER;
    game->player1_word[0] = '\0';
    game->player2_word[0] = '\0';
    game->player1_guesses = 0;
    game->player2_guesses = 0;
    game->player1_solved  = false;
    game->player2_solved  = false;
 
    return game;
}
 
void add_player(Player *player, Game *game) {
    game->player2 = player;
    game->state   = WAITING_FOR_WORDS;
}
 
int set_word(Game *game, Player *player, const char *word) {
    if (!is_valid_word(word)) return 0;
 
    if (player == game->player1)
        lowercase_word(game->player1_word, word);
    else if (player == game->player2)
        lowercase_word(game->player2_word, word);
    else
        return 0;
 
    // start game once both words are in
    if (game->player1_word[0] != '\0' && game->player2_word[0] != '\0')
        game->state = IN_PROGRESS;
 
    return 1;
}
 
bool check_guess(Game *game, Player *player, const char *guess, int result[WORD_LENGTH]) {
    // player1 guesses player2's word and vice versa
    const char *target = (player == game->player1) ? game->player2_word : game->player1_word;
 
    char lower[WORD_LENGTH + 1];
    lowercase_word(lower, guess);
 
    // two-pass scoring: greens first, then yellows
    int target_used[WORD_LENGTH] = {0};
    int guess_green[WORD_LENGTH] = {0};
 
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (lower[i] == target[i]) {
            result[i] = 2;
            target_used[i] = guess_green[i] = 1;
        } else {
            result[i] = 0;
        }
    }
 
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (guess_green[i]) continue;
        for (int j = 0; j < WORD_LENGTH; j++) {
            if (!target_used[j] && lower[i] == target[j]) {
                result[i] = 1;
                target_used[j] = 1;
                break;
            }
        }
    }
 
    bool solved = (strncmp(lower, target, WORD_LENGTH) == 0);
 
    if (player == game->player1) {
        game->player1_guesses++;
        if (solved) game->player1_solved = true;
    } else {
        game->player2_guesses++;
        if (solved) game->player2_solved = true;
    }
 
    return solved;
}
 
bool is_game_over(Game *game) {
    bool p1_done = game->player1_solved || (game->player1_guesses >= MAX_GUESSES);
    bool p2_done = game->player2_solved || (game->player2_guesses >= MAX_GUESSES);
    return p1_done && p2_done;
}
 
void finalize_scores(Game *game) {
    // score = guesses remaining if solved, 0 if not
    game->player1_score = game->player1_solved ? (MAX_GUESSES - game->player1_guesses) : 0;
    game->player2_score = game->player2_solved ? (MAX_GUESSES - game->player2_guesses) : 0;
    game->state = GAME_OVER;
}
 
void end_game(Game *game) {
    free(game);
}
