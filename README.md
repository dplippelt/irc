# ft_irc
A non-blocking IRC server implemented in C++, built as part of the Codam (42 Network) curriculum. The server handles multiple simultaneous clients using a `poll()`-based event loop and communicates over TCP/IP.

## Features
### Server

- Password-protected connections
- Client registration via `PASS`, `NICK`, and `USER`
- Channel creation and membership management
- Public and private messaging (`PRIVMSG`)
- `WHOIS` support
- CTCP / DCC handling
- Operator commands: `KICK`, `INVITE`, `TOPIC`, `MODE`
- Channel modes: invite-only (`i`), topic restriction (`t`), key (`k`), user limit (`l`), operator privilege (`o`)

### Bot — BattleshipsBot

- Single-player Battleships against the bot (target practice)
- Multiplayer: challenge other users with `!challenge`, accept with `!accept`
- In-channel file transfer: `!file <filename>` serves game info files to users
- Game instructions are sent in the `#Battleships` channel upon first time joining
- Use `!help` to see a full list of Battleships commands


## Architecture
### Server
The server uses a single `poll()` loop for all I/O with non-blocking file descriptors (FDs). This allows it to monitor activity across numerous connections, avoiding the overhead and added complexity of forking or multithreading.  
Following IRC protocol a full message is required to end in `\r\n`. Whenever a client sends data to the server it will trigger a POLLIN event for the FD associated with that client. In case the server receives an incomplete message (i.e. not ending in `\r\n`), the remainder is stored in an FD-matched receive-buffer and prepended to the next message received from that client. Similarly, partially sent messages from the server to a client are saved to a FD-matched send-buffer and the POLLOUT event flag is set for the FD to signal there is remaining data ready to be sent to the client.  
Whenever data is received by the server the Parser class extracts messages and saves remainders based on the `\r\n`-delimiter. Messages are then forwarded to the Command class for execution. Command validation is handled by a dedicated Validator class, extracted from the command handlers after the combined approach produced unwieldy code that was difficult to reason about.

### Bot
The bot simulates a client connection to the server. To this end it uses a the same approach as the server, using a single `poll()` loop. On startup the bot connects to the server with the username `BattleshipsBot` and joins a channel called `#Battleships`. It has its own custom `!`-prepended command system (e.g. `!help`) allowing users to interact with the bot while in its channel or through direct messages (`PRIVMSG`). Upon first entering the channel the user is greeted by the bot with a welcome message and basic usage instructions.  
Furthermore, the bot keeps track of currently running games and challenges. For instance, when a multiplayer command is issued (e.g. `!shoot bob A2`) the bot checks whether an active game exists for the user and the target before executing the command. If not, it will instead provide helpful feedback on how to start a game.


## Usage
Requirements: `make`, `c++`
```
git clone git@github.com:dplippelt/irc.git
cd irc
make                          # builds both ircserv and ircbot
make server                   # builds only ircserv
make bot                      # builds only ircbot
./ircserv <port> <password>   # starts the irc server at the specified port with the specified password
./ircbot  <port> <password>   # starts the Battleships bot (port and password need to match the server's parameters)
```
Connect using any IRC client (tested with `irssi`) to `localhost:<port>` with the password you provided.
If the Battleships bot is running join `#Battleships` to interact with it.


## Team
[Dominique Lippelt](https://github.com/dplippelt) (server setup, command validation, battleships bot)  
[Seungah Pyun](https://github.com/seungahpyun) (user setup, channel setup, command execution and validation)  
[Takato Mitsuya](https://github.com/takato7) (message parsing, `mode` command execution and validation)  

