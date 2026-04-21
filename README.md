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

