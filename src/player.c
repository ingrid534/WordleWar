#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "player.h"

Player *init_player(int fd) {
    Player *player = malloc(sizeof(Player));
    if (player == NULL) {
        return NULL;
    }

    player->fd = fd;
    player->game = NULL;
    player->state = WAITING_NAME;
    player->name[0] = '\0';
    player->word[0] = '\0';
    player->board[0] = '\0';
    player->buf[0] = '\0';
    player->inbuf = 0;
    return player;
}

void update_word(Player *player, const char *word) {
    strncpy(player->word, word, MAX_WORD_LENGTH);
    player->word[MAX_WORD_LENGTH] = '\0';
}

void update_board(Player *player, const char *board) {
    strncpy(player->board, board, MAX_WORD_LENGTH);
    (player->board)[MAX_WORD_LENGTH] = '\0';
}

void remove_player(Player *player) {
    if (player != NULL) {
        free(player);
    }
}