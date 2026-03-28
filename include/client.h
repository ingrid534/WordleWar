#ifndef _CLIENT_H_
#define _CLIENT_H_

int connect_to_server(int soc, int port, const char *hostname);
void give_name(int soc);
void give_game_choice(int soc);
void give_word(int soc);
void guess_letter(int soc);
char game_settings(int soc);
char * read_server_msg(int soc);
char * validate_game_entries(int soc, char *line_read, char *msg, char *user_prompt);
void write_to_server(int soc, char *msg, int msg_buffer_size);
int check_exit_status(char *line);
void check_guess(char *line);

#endif