// shared communication signals between server and client
#ifndef PROTOCOL_H
#define PROTOCOL_H

// signals for game initialization
#define NAME "name"
#define CHOICE "choice"
#define CODE "code"
#define CMD_CODE "code:"
#define WORD   "word"
#define GAME_CODE "Session game code:" // we need to give user game code if they created a game so it can be shared
#define INVALID_CODE "wrong code" // signal that user entered an invalid code
#define INVALID_WORD "bad_word" // signal that user entered an invalid dictionary word
#define GAME_FULL "game_full" // signal when server has no capacity for new games


// signals for game play
#define BOARD "board:" // signal to display board and prompt user for guess

// Why do we need guessed word - at that point, you'd likely send wait?
#define GUESSED_WORD "guessed" // signal to tell user they guessed their word
#define LENGTH "bad_length" // signal to tell the user their input is not same length as their word
#define OUT_OF_GUESSES "out_of_guesses" // signal to tell the user they used all guesses

// signals for end game
#define STAT_WAIT  "wait"
#define STAT_WIN   "win:"
#define STAT_LOST  "lost:"
#define STAT_LOSE  STAT_LOST

// data
#define BUFSIZE 256

#endif // PROTOCOL_H