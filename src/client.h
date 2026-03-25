#ifndef _CLIENT_H_
#define _CLIENT_H_

int connect_to_server(int soc, int port, const char *hostname);
void give_word(int soc);
void guess_letter(int soc);
char * read_server_msg(int soc);


#endif