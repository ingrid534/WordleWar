// shared communication signals between server and client
#ifndef PROTOCOL_H
#define PROTOCOL_H

// signals for game initialization
#define NAME "name"
#define CHOICE "choice"
#define CODE "code"
#define CMD_CODE "code:"
#define WORD "word"
#define GAME_CODE "Session game code:" 
#define INVALID_CODE "wrong code" 
#define INVALID_WORD "bad_word" 
#define GAME_FULL "game_full" 


// signals for game play
#define BOARD "board:" 
#define WAIT_OPPONENT_WORD "wait_opponent_word" 

#define GUESSED_WORD "guessed" 
#define LENGTH "bad_length" 
#define OUT_OF_GUESSES "out_of_guesses" 

// signals for end game
#define STAT_WAIT "wait"
#define STAT_WIN "win:"
#define STAT_LOST "lost:"
#define STAT_TIE "tie:"

// data
#define BUFSIZE 256

#endif // PROTOCOL_H