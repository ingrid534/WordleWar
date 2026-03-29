// shared communication signals between server and client
#ifndef PROTOCOL_H
#define PROTOCOL_H

// signals for game initialization
#define NAME "name"
#define CHOICE "choice"
#define CODE "code"
#define WORD   "word"

// signals for game play
#define BOARD "board:" // signal to display board and prompt user for guess
#define GUESSED_WORD "guessed" // signal to tell user they guessed their word
#define LENGTH "bad_length" // signal to tell the user their input is not same legnth as their word

// signals for end game
#define STAT_WAIT  "wait"
#define STAT_WIN   "win:"
#define STAT_LOSE  "lost:"

// data
#define BUFSIZE 20

#endif // PROTOCOL_H