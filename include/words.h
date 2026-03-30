// words.h - load word bank and validate words
#ifndef WORDS_H
#define WORDS_H
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
 
#define MAX_WORDS       6000
#define MIN_WORD_LENGTH 5
#define MAX_WORD_LENGTH 8
 
// global word bank populated by load_word_bank()
extern char word_bank[MAX_WORDS][MAX_WORD_LENGTH + 1];
extern int  word_count;
 
void load_word_bank(const char *filename); // reads words from file, skips invalid lines
int  is_valid_word(const char *word);      // checks word is alpha, 5-8 chars, and in bank
void free_words(void);                     // resets word_count
 
#endif // WORDS_H
