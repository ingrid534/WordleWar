#ifndef _CLIENT_H_
#define _CLIENT_H_

int connect_to_server(int soc, int port, const char *hostname);
void prompt_name(int soc);
void give_game_choice(int soc);
void prompt_code(int soc);
void prompt_word(int soc);
void prompt_guess(int soc, const char *server_msg);
void give_correct_word(int soc, const char *server_msg);
void give_wait(int soc);
void give_win(int soc, const char *server_msg);
void give_lost(int soc, const char *server_msg);
void give_tie (int soc, const char *server_msg);
char * read_server_msg(int soc);
void write_to_server(int soc, char *msg, int msg_buffer_size);

#endif