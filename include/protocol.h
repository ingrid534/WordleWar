// shared communication signals between server and client
#ifndef PROTOCOL_H
#define PROTOCOL_H

// signals for game initialization
#define NAME "name"
#define CHOICE "choice"
#define CODE "code"
#define WORD   "word"

// signals for game play
#define GUESS  "guess"
#define GUESSED_WORD "guessed"
#define BOARD "board:"

// signals for end game
#define STAT_WAIT  "wait"
#define STAT_WIN   "win:"
#define STAT_LOSE  "lost:"

#endif // PROTOCOL_H