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
 
    // player1 chooses the word player2 has to guess, and vice versa
    if (player == game->player1)
        lowercase_word(game->player2_word, word);
    else if (player == game->player2)
        lowercase_word(game->player1_word, word);
    else
        return 0;
 
    // start game once both words are in
    if (game->player1_word[0] != '\0' && game->player2_word[0] != '\0')
        game->state = IN_PROGRESS;
 
    return 1;
}
 
bool check_correct_word(const char *guess, const char *target) {
    return strncmp(guess, target, WORD_LENGTH) == 0;
}
 
// returns a malloc'd WORD_LENGTH+1 string:
//   '-'       = letter not in word
//   lowercase = right letter, wrong position
//   UPPERCASE = right letter, right position
char *check_guess(Game *game, Player *player, const char *guess) {
    const char *target = (player == game->player1) ? game->player1_word : game->player2_word;
 
    char lower[WORD_LENGTH + 1];
    lowercase_word(lower, guess);
 
    char *result = malloc(WORD_LENGTH + 1);
    if (!result) { perror("check_guess: malloc"); exit(1); }
 
    // two-pass scoring: greens first, then yellows
    int target_used[WORD_LENGTH] = {0};
    int guess_green[WORD_LENGTH] = {0};
 
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (lower[i] == target[i]) {
            result[i]      = (char)toupper((unsigned char)lower[i]);
            target_used[i] = guess_green[i] = 1;
        } else {
            result[i] = '-';
        }
    }
 
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (guess_green[i]) continue;
        for (int j = 0; j < WORD_LENGTH; j++) {
            if (!target_used[j] && lower[i] == target[j]) {
                result[i]      = lower[i]; // lowercase = right letter, wrong spot
                target_used[j] = 1;
                break;
            }
        }
    }
    result[WORD_LENGTH] = '\0';
 
    bool solved = check_correct_word(lower, target);
 
    if (player == game->player1) {
        game->player1_guesses++;
        if (solved) game->player1_solved = true;
    } else {
        game->player2_guesses++;
        if (solved) game->player2_solved = true;
    }
 
    return result;
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
