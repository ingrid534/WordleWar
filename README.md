# WordleWar
 
A two-player, real-time Wordle game built as a TCP client-server application in C. Two players connect to a shared server, each picks a secret word for the other to guess, then both race to solve their opponent's word in the fewest guesses. The server handles multiple concurrent game sessions using a non-blocking `select()`-based event loop.
 
---
 
## Table of Contents
 
- [About](#about)
- [Usage](#usage)
- [Development](#development)
  - [File Structure](#file-structure)
  - [Communication Protocol](#communication-protocol)
---
 
## About
 
WordleWar is a competitive twist on the classic Wordle formula. When you connect to the server, you're placed in a lobby where you can either create a new game (which generates a four-digit join code to share with a friend) or join an existing game by entering a code.
 
Once both players are seated in a game session, each player privately selects a secret word (5–8 letters) from a shared bank of 5,459 valid English words. That word becomes the target the opponent must guess. Both players then guess simultaneously and independently — you won't need to wait for your opponent between turns.
 
After each guess, the server returns a result string using Wordle-style feedback:
 
```
Uppercase letter  →  correct letter, correct position
lowercase letter  →  correct letter, wrong position
-                 →  letter not in the word
 
Example: "-A-le-" for a six-letter target
```
 
Each player has up to **20 guesses** to solve their word. The player who solves their word in fewer guesses wins the round. If both players fail to guess within the limit, both lose.
 
All interaction is text-based via the terminal. The server supports up to **64 concurrent game sessions** (128 players). If a player disconnects mid-game, their opponent is returned to the lobby.
 
---
 
## Usage
 
### Getting the Project
 
Clone the repository from GitHub:
 
```bash
git clone https://github.com/<your-username>/WordleWar.git
cd WordleWar
```
 
### Building
 
Build both the server and client executables with:
 
```bash
make
```
 
This produces two binaries: `wordle` (server) and `wordle_client` (client).
 
### Running
 
Start the server:
 
```bash
./wordle
```
 
In a separate terminal (or on another machine), connect a client:
 
```bash
./wordle_client <server-ip-address>
```
 
Replace `<server-ip-address>` with the IP address of the machine running the server. Two clients must connect and join the same game code to play.
 
---
 
## Development
 
### File Structure
 
```
WordleWar/
├── include/
│   ├── client.h
│   ├── game.h
│   ├── player.h
│   ├── protocol.h
│   ├── server.h
│   └── words.h
├── src/
│   ├── client.c
│   ├── game.c
│   ├── player.c
│   ├── server.c
│   └── words.c
├── Makefile
├── proposal.pdf
└── word.txt
```
 
| File | Purpose |
|------|---------|
| `server.c / server.h` | Main server loop, `select()`-based concurrency, connection handling |
| `client.c / client.h` | Client-side I/O, user prompts, server message parsing |
| `game.c / game.h` | Game session logic, guess scoring (two-pass algorithm), win/loss resolution |
| `player.c / player.h` | Player state management and data structures |
| `words.c / words.h` | Word bank loading, validation against the dictionary |
| `protocol.h` | Shared protocol constants and signal definitions used by both server and client |
| `word.txt` | Dictionary of 5,459 valid 5–8 letter English words |
| `Makefile` | Build rules for the `wordle` and `wordle_client` executables |
 
### Communication Protocol
 
All messages are plain ASCII strings sent over TCP, terminated by `\r\n`. The server drives the conversation by sending a **signal keyword**; the client responds with user input. Both sides buffer incoming bytes and extract complete messages before processing.
 
The protocol follows a strict request-response flow through four phases:
 
**1. Lobby / Setup**
 
```
Server              Client
  |--- "name" -------->|  (client sends back username)
  |--- "choice" ------->|  (client sends "C" to create or "J" to join)
  |--- "code" --------->|  (if joining: client sends 4-digit code)
  |--- "code:XXXX" ---->|  (if creating: server echoes the generated code)
```
 
**2. Word Selection**
 
```
Server              Client
  |--- "word" --------->|  (client sends a 5-8 letter word)
  |                      |  (server validates against word bank)
  |--- "invalid word" ->|  (re-prompts if word is invalid)
```
 
**3. Gameplay**
 
```
Server                        Client
  |--- "board:-A-le-:5" ------->|  (result string + guesses remaining)
  |                              |  (client sends next guess)
  |--- "bad_length" ----------->|  (guess had wrong length, no penalty)
  |--- "guessed" -------------->|  (player solved the word)
  |--- "out_of_guesses" ------->|  (player used all 20 guesses)
```
 
**4. End Game**
 
```
Server              Client
  |--- "wait" -------->|  (opponent still guessing)
  |--- "win:8" -------->|  (you won with this score)
  |--- "lost:3" ------->|  (you lost with this score)
  |--- "tie:5" -------->|  (tied scores)
  |--- "failed" ------->|  (failed to guess within limit)
```
 
Scores are calculated as `guesses_used`. The player with the lowest `guesses_used` wins. If a client disconnects mid-game, the server sends `"player disconnected"` to the remaining player and returns them to the lobby.
