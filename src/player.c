#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "PLAYER_H"

Player *init_player(int fd) {
    Player *player = malloc(sizeof(Player));
    player->fd = fd;
    player->state = WAITING_NAME;
    player->inbuf = 0;
    return player;
}

void update_word(Player *player, char *word) {
    strncpy(player->word, word, MAX_WORD_LENGTH);
    player->word[MAX_WORD_LENGTH] = '\0';
}

void update_board(Player *player, char *board) {
    strncpy(player->board, board, MAX_WORD_LENGTH);
    (player->board)[MAX_WORD_LENGTH] = '\0';
}

void remove_player(Player *player) {
    if (player != NULL) {
        free(player;)
    }
}