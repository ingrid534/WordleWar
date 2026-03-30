// load word bank and validate words
#ifndef WORDS_H
#define WORDS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORDS 1000
#define MAX_WORD_LENGTH 8

extern char words[MAX_WORDS][MAX_WORD_LENGTH];

void load_word_bank(const char *filename);
int is_valid_word(const char *word);
void free_words(char **words);

#endif
