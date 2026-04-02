FLAGS = -Wall -Wextra -g -Iinclude -DPORT=43465

target = wordle
client_target = wordle_client

server_obj = src/server.o src/game.o src/player.o src/words.o
client_obj = src/client.o

.PHONY: clean all run client

all: $(target) $(client_target)

$(target): $(server_obj)
	gcc $(FLAGS) -o $@ $^

$(client_target): $(client_obj)
	gcc $(FLAGS) -o $@ $^

client: $(client_target)

src/server.o: include/server.h include/player.h include/game.h include/protocol.h
src/game.o: include/game.h include/player.h include/words.h
src/player.o: include/player.h include/protocol.h
src/words.o: include/words.h
src/client.o: include/client.h include/protocol.h

src/%.o: src/%.c
	gcc $(FLAGS) -c $< -o $@

run: $(target)
	./$(target)

clean:
	rm -f $(target) $(client_target) src/*.o
