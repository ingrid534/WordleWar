#include "words.h"
 
char word_bank[MAX_WORDS][MAX_WORD_LENGTH + 1];
int  word_count = 0;
 
void load_word_bank(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("load_word_bank: fopen");
        exit(1);
    }
 
    char line[64];
    word_count = 0;
 
    while (word_count < MAX_WORDS && fgets(line, sizeof(line), f)) {
        // strip newline
        int len = (int)strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            line[--len] = '\0';
 
        // skip words outside the valid length range
        if (len < MIN_WORD_LENGTH || len > MAX_WORD_LENGTH) continue;
 
        // skip non-alpha words, lowercase the rest
        int valid = 1;
        for (int i = 0; i < len; i++) {
            if (!isalpha((unsigned char)line[i])) { valid = 0; break; }
            line[i] = (char)tolower((unsigned char)line[i]);
        }
        if (!valid) continue;
 
        strncpy(word_bank[word_count++], line, MAX_WORD_LENGTH + 1);
    }
 
    fclose(f);
}
 
// linear search through word bank
int is_valid_word(const char *word) {
    if (!word) return 0;
 
    int len = (int)strlen(word);
    if (len < MIN_WORD_LENGTH || len > MAX_WORD_LENGTH) return 0;
 
    // check all alpha
    for (int i = 0; i < len; i++)
        if (!isalpha((unsigned char)word[i])) return 0;
 
    // check it's actually in the bank
    char lower[MAX_WORD_LENGTH + 1];
    for (int i = 0; i < len; i++)
        lower[i] = (char)tolower((unsigned char)word[i]);
    lower[len] = '\0';
 
    for (int i = 0; i < word_count; i++)
        if (strcmp(word_bank[i], lower) == 0) return 1;
 
    return 0;
}
 
void free_words(void) {
    word_count = 0;
}
