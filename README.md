This project is a networked Escape Room game implemented in C using TCP sockets.
It is based on a client-server architecture where the server manages the game logic and the clients interact with it by sending commands.

# Features

Communication over TCP for reliable message delivery.

Message protocol: first send message size (binary), then message content (text).

Commands sent as 1-byte operation codes (defined in cmdClientServer.h).

Authentication system with registration (utenti.txt stores users).

Two client roles:

#Player – plays the escape room.

#Helper – sends random bonus time to players.

#Server uses I/O multiplexing to manage multiple clients efficiently.

Game logic implemented as a finite state machine (FSM):

Player state changes based on puzzles solved and objects used.

Objects and locations also have states (e.g., locked/unlocked).

Server tracks remaining time and tokens collected.

#How It Works

Start the server:
Run and type start to begin listening for clients.

Clients connect:
They authenticate or register, select their role and the room (e.g. “F6”).

Gameplay:

Players type commands (look, take, use, drop, etc.).

The server checks validity, updates state, and sends results.

End of game:
The server notifies when time expires or the escape is complete.

Files

server.c – main server logic and client handling.

client.c – player-side command input and message sending.

cmdClientServer.h – command byte definitions.

gameF6.h – data structures and constants for the F6 room.

utenti.txt – stores registered user data.

#Requirements

GCC compiler

POSIX-compatible system (Linux recommended)

#Run Example

gcc server.c -o server
gcc client.c -o client

./server

Run client (in another terminal)
./client
