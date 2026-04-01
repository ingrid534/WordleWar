#ifndef _CLIENT_H_
#define _CLIENT_H_

int connect_to_server(int soc, int port, const char *hostname);
int prompt_name(int soc);
int give_game_choice(int soc);
int prompt_code(int soc);
int prompt_word(int soc);
int prompt_guess(int soc, const char *server_msg);
void give_correct_word(int soc, const char *server_msg);
void give_wait(int soc);
void give_win(int soc, const char *server_msg);
void give_lost(int soc, const char *server_msg);
void give_tie (int soc, const char *server_msg);
int give_disconnect(int soc);
char * read_server_msg(int soc);
int write_to_server(int soc, char *msg, int msg_buffer_size);

#endif