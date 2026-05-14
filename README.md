# Socket Tic-Tac-Toe

A classic Tic-Tac-Toe game implemented in C using BSD sockets for multiplayer functionality and `ncurses` for a Terminal User Interface (TUI).

## Table of Contents
- [Features](#features)
- [Architecture & Implementation](#architecture--implementation)
- [Requirements](#requirements)
- [Building the Project](#building-the-project)
- [How to Run](#how-to-run)
- [How to Play](#how-to-play)
- [Patterns Used](#patterns-used)

## Features
- **Multiplayer:** Play with friends over a network.
- **Customizable Board:** Create games with board sizes ranging from 3x3 to 10x10.
- **Custom Player Data:** Choose your name and any 1-3 character shape.
- **TUI:** Interactive terminal interface with arrow key navigation.
- **Concurrent Games:** Server supports multiple simultaneous game sessions.

## Architecture & Implementation
The project follows a **Client-Server architecture** using TCP sockets.

### Server
- **Asynchronous I/O:** Uses the `select()` system call to handle multiple client connections and game sessions within a single thread.
- **Game Management:** Handles game creation, joining, and validation of moves.
- **Protocol:** Uses a simple line-based text protocol for communication.

### Client
- **Multi-threaded:** A dedicated background thread handles socket communication, ensuring the UI remains responsive.
- **TUI:** Built using the `ncurses` library for a rich terminal experience.
- **State Machine:** Manages different phases of the game (Setup, Awaiting Join, In Progress, Finished).

## Requirements
- **Compiler:** `gcc`
- **Build System:** `make`
- **Libraries:**
  - `ncurses` (for the client UI)
  - `lpthread` (for client-side multi-threading)

On Debian/Ubuntu, you can install the dependencies with:
```bash
sudo apt-get install build-essential libncurses5-dev libncursesw5-dev
```

## Building the Project
To compile both the server and the client, run:
```bash
make all
```
The binaries will be generated in the `bin/` directory:
- `bin/ttt-server`
- `bin/ttt-client`

## How to Run

### 1. Start the Server
Run the server by specifying a port:
```bash
./bin/ttt-server <port>
# Example:
./bin/ttt-server 8080
```

### 2. Start the Clients
Open two separate terminal windows and start a client in each:
```bash
./bin/ttt-client <server-ip> <port>
# Example (if running locally):
./bin/ttt-client 127.0.0.1 8080
```

## How to Play
1. **Connect:** Both players connect to the server using the client.
2. **Setup:** Each player enters their name and preferred shape (e.g., "X", "O", or even "☺").
3. **Create/Join:**
   - One player chooses **"Create a new game"** and specifies the board size. The server will provide a **4-digit game code**.
   - The second player chooses **"Join a game"** and enters the code provided by the first player.
4. **Gameplay:**
   - Use the **Arrow Keys** to move the cursor between cells.
   - Press **Enter** to place your shape in the selected cell.
   - The game will automatically detect winners or a draw.

## Patterns Used
- **Reactor Pattern (Subset):** The server uses `select()` to demultiplex events from multiple socket handles, similar to the Reactor pattern.
- **Client-Server:** Clear separation between the game's authoritative state (server) and its presentation/input (client).
- **Producer-Consumer (Internal):** The client's socket thread produces messages from the server, which are then consumed by the main game loop thread using mutexes and condition variables.
