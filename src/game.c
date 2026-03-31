#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"

static Game *games[MAX_GAMES] = {0};

static void lowercase_word(char *dst, const char *src, int length) {
    for (int i = 0; i < length; i++) {
        dst[i] = (char)tolower((unsigned char)src[i]);
    }
    dst[length] = '\0';
}

static const char *target_for_player(Game *game, Player *player, int *length) {
    if (player == game->player1) {
        *length = game->player1_word_length;
        return game->player1_word;
    }

    *length = game->player2_word_length;
    return game->player2_word;
}

static int generate_join_code(void) {
    for (int attempts = 0; attempts < 10000; attempts++) {
        int code = (rand() % 9000) + 1000;
        if (find_game_by_code(code) == NULL) {
            return code;
        }
    }
    return 1000;
}

Game *init_game(Player *player1) {
    int slot = -1;
    for (int i = 0; i < MAX_GAMES; i++) {
        if (games[i] == NULL) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        return NULL;
    }

    Game *game = malloc(sizeof(Game));
    if (game == NULL) {
        perror("init_game: malloc");
        exit(1);
    }

    memset(game, 0, sizeof(Game));
    game->player1 = player1;
    game->join_code = generate_join_code();
    game->state = WAITING_FOR_PLAYER;

    player1->game = game;

    games[slot] = game;

    return game;
}

Game *find_game_by_code(int code) {
    for (int i = 0; i < MAX_GAMES; i++) {
        if (games[i] != NULL && games[i]->join_code == code) {
            return games[i];
        }
    }
    return NULL;
}

void add_player(Player *player, Game *game) {
    game->player2 = player;
    player->game = game;

    // If player1 already submitted player2's target, initialize late joiner state.
    if (game->player2_word_length > 0) {
        update_word(player, game->player2_word);
        memset(player->board, '-', (size_t)game->player2_word_length);
        player->board[game->player2_word_length] = '\0';
    }

    game->state = WAITING_FOR_WORDS;
}

int set_word(Player *player, const char *word) {
    if (player == NULL || player->game == NULL || !is_valid_word(word)) {
        return 0;
    }

    Game *game = player->game;
    int len = (int)strlen(word);

    // Player submits the word that the opponent must guess.
    if (player == game->player1) {
        lowercase_word(game->player2_word, word, len);
        game->player2_word_length = len;
        if (game->player2 != NULL) {
            update_word(game->player2, game->player2_word);
            memset(game->player2->board, '-', len);
            game->player2->board[len] = '\0';
        }
    } else if (player == game->player2) {
        lowercase_word(game->player1_word, word, len);
        game->player1_word_length = len;
        update_word(game->player1, game->player1_word);
        memset(game->player1->board, '-', len);
        game->player1->board[len] = '\0';
    } else {
        return 0;
    }

    if (game->player1_word_length > 0 && game->player2_word_length > 0) {
        game->state = IN_PROGRESS;
    }

    return 1;
}

bool check_correct_word(Player *player, const char *guess) {
    if (player == NULL || player->game == NULL || guess == NULL) {
        return false;
    }

    Game *game = player->game;
    int length = 0;
    const char *target = target_for_player(game, player, &length);

    if (length <= 0) {
        return false;
    }

    char lower[MAX_WORD_LENGTH + 1];
    lowercase_word(lower, guess, length);
    return strncmp(lower, target, (size_t)length) == 0;
}

char *check_guess(Player *player, const char *guess) {
    if (player == NULL || player->game == NULL || guess == NULL) {
        return NULL;
    }

    Game *game = player->game;
    int length = 0;
    const char *target = target_for_player(game, player, &length);

    if (length <= 0 || length > MAX_WORD_LENGTH) {
        return NULL;
    }

    char lower[MAX_WORD_LENGTH + 1];
    lowercase_word(lower, guess, length);

    char *result = malloc((size_t)length + 1);
    if (result == NULL) {
        perror("check_guess: malloc");
        exit(1);
    }

    int target_used[MAX_WORD_LENGTH] = {0};
    int guess_green[MAX_WORD_LENGTH] = {0};

    for (int i = 0; i < length; i++) {
        if (lower[i] == target[i]) {
            result[i] = (char)toupper((unsigned char)lower[i]);
            target_used[i] = 1;
            guess_green[i] = 1;
        } else {
            result[i] = '-';
        }
    }

    for (int i = 0; i < length; i++) {
        if (guess_green[i]) {
            continue;
        }
        for (int j = 0; j < length; j++) {
            if (!target_used[j] && lower[i] == target[j]) {
                result[i] = lower[i];
                target_used[j] = 1;
                break;
            }
        }
    }
    result[length] = '\0';

    if (player == game->player1) {
        game->player1_guesses++;
        if (check_correct_word(player, guess)) {
            game->player1_solved = true;
        }
    } else {
        game->player2_guesses++;
        if (check_correct_word(player, guess)) {
            game->player2_solved = true;
        }
    }

    return result;
}

bool is_game_over(Game *game) {
    bool p1_done = game->player1_solved || (game->player1_guesses >= MAX_GUESSES);
    bool p2_done = game->player2_solved || (game->player2_guesses >= MAX_GUESSES);
    return p1_done && p2_done;
}

void finalize_scores(Game *game) {
    game->player1_score = game->player1_solved ? (MAX_GUESSES - game->player1_guesses) : 0;
    game->player2_score = game->player2_solved ? (MAX_GUESSES - game->player2_guesses) : 0;
    game->state = GAME_OVER;
}

void end_game(Game *game) {
    if (game == NULL) {
        return;
    }

    for (int i = 0; i < MAX_GAMES; i++) {
        if (games[i] == game) {
            games[i] = NULL;
            break;
        }
    }
    free(game);
}
