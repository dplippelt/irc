/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 10:39:01 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/25 16:18:35 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bot.hpp"

/* ==================== Constructors & Destructors ==================== */

Bot::~Bot()
{
	if (m_server_addr)
		freeaddrinfo(m_server_addr);
	for (auto game : m_games)
		delete game.second;
}

Bot::Bot( const std::string& server_address, const std::string& server_port, std::string_view pw )
	: m_pw { pw }
{
	struct addrinfo	hints { 0, AF_INET, SOCK_STREAM, 0, 0, NULL, NULL, NULL };

	validatePort(server_port);
	validateAddress(server_address);

	if (getaddrinfo(server_address.data(), server_port.data(), &hints, &m_server_addr))
		throw std::runtime_error("Error: failed to get required address info");
	if ( m_server_addr->ai_next != NULL )
		std::cerr << "Warning: found more than one matching set of address info";

	m_bot_socket_fd = socket(m_server_addr->ai_family, m_server_addr->ai_socktype, m_server_addr->ai_protocol);

	if ( m_bot_socket_fd == - 1)
		throw std::runtime_error("Error: failed to create socket");

	if ( connect(m_bot_socket_fd, m_server_addr->ai_addr, m_server_addr->ai_addrlen) )
		throw std::runtime_error("Error: could not connect to server");

	if ( fcntl(m_bot_socket_fd, F_SETFL, O_NONBLOCK) == -1 )
		throw std::runtime_error("Error: failed to set bot socket to non-blocking");

	m_pollfd.fd = m_bot_socket_fd;
	m_pollfd.events = POLLIN;

	freeaddrinfo(m_server_addr);
	m_server_addr = nullptr;

	authenticateAndJoin();
}





/* ==================== Initial Server Setup ==================== */

void	Bot::validatePort( const std::string& port ) const
{
	int	port_int {};

	for ( char c : port )
	{
		if ( !std::isdigit(static_cast<unsigned char>(c)) )
			throw std::runtime_error("Error: port '" + port + "' contains non-digit characters");
	}

	try
	{
		port_int = std::stoi(port);
	}
	catch ( const std::exception& e )
	{
		throw std::runtime_error("Error: could not convert port '" + port + "' to integer");
	}

	if ( port_int < 1 || port_int > 65535 )
		throw std::runtime_error("Error: port '" + port + "' must be between 1 and 65535");
}

void	Bot::validateAddress( const std::string& address ) const
{
	if (address == "localhost")
		return;

	int					noctect {};
	int					octet_num {};
	std::string			octet_str {};
	std::istringstream	iss { address };

	while ( std::getline(iss, octet_str, '.') )
	{
		try
		{
			octet_num = std::stoi(octet_str);
		}
		catch ( const std::exception& e )
		{
			throw std::runtime_error("Error: could not convert address octect '" + octet_str + "' to integer");
		}

		if ( octet_num < 0 || octet_num > 255 )
			throw std::runtime_error("Error: octect '" + octet_str + "' must be between 0 and 255");

		noctect++;
	}

	if ( address.at(address.length() - 1) == '.' )
		throw std::runtime_error("Error: the server address '" + address + "' cannot end with a '.'");

	if ( noctect != 4 )
		throw std::runtime_error("Error: the server address '" + address + "' contains an invalid number of octets");
}

void	Bot::authenticateAndJoin() const
{
	std::string pass_cmd { "PASS " + m_pw + "\r\n" };
	std::string nick_cmd { "NICK BattleshipsBot\r\n" };
	std::string user_cmd { "USER bot_user 0 * :BattleshipsBot\r\n" };
	std::string join_cmd { "JOIN #Battleships\r\n" };

	send(m_bot_socket_fd, pass_cmd.data(), pass_cmd.length(), 0);
	send(m_bot_socket_fd, nick_cmd.data(), nick_cmd.length(), 0);
	send(m_bot_socket_fd, user_cmd.data(), user_cmd.length(), 0);
	send(m_bot_socket_fd, join_cmd.data(), join_cmd.length(), 0);
}

void	Bot::doPoll()
{
	#ifdef DEBUG
	std::cout << "Polling..." << std::endl;
	#endif

	int ret = poll(&m_pollfd, 1, -1);

	if ( ret == -1 )
	{
		switch (errno)
		{
		case EINTR:
			return;
		case EINVAL:
			throw std::runtime_error("Error: poll called with invalid argument");
		case ENOMEM:
			throw std::runtime_error("Error: failed to allocate memory");
		case EFAULT:
			throw std::runtime_error("Error: file descriptor(s) out of range");
		default:
			throw std::runtime_error("Error: an unknown error occurred during polling");
		}
	}

	if ( m_pollfd.revents & (POLLHUP | POLLERR) )
		throw std::runtime_error("Connection closedn\n");

	if ( m_pollfd.revents & POLLIN )
		receiveMessage();
}

void	Bot::receiveMessage()
{
	#ifdef DEBUG
	std::cout << "Activity detected on bot socket!" << std::endl;
	#endif

	char buffer[512];
	ssize_t bytes = recv(m_bot_socket_fd, buffer, sizeof(buffer) - 1, 0);

	if ( bytes == 0 )
	{
		throw std::runtime_error("Server disconnected");
	}
	if ( bytes == -1 )
	{

		if ( errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR )
			return;

		throw std::runtime_error("Error while receiving message from server. Disconnecting bot...");
	}

	buffer[bytes] = '\0';

	#ifdef DEBUG
	std::cout << "Bot received:\n" << buffer << std::endl;
	#endif

	processBuffer(buffer);
}





/* ==================== Message Processing ==================== */

void	Bot::processBuffer( const std::string& buffer )
{
	std::string username { getUserName(buffer) };
	std::string message { getMessage(buffer) };
	std::string channel { getChannelName(buffer) };
	std::string target {};

	#ifdef DEBUG
	std::cout << "Sender: " << username << std::endl;
	std::cout << "Channel: " << channel << std::endl;
	std::cout << "Message: " << message << std::endl;
	std::cout << "Message length: " << message.length() << std::endl;
	#endif

	std::string cmd { message.substr(0, message.find_first_of(" ")) };

	switch ( getCmdType(cmd) )
	{
	case CMD_START:
		if (message.length() == e_start)
			startGame(username, channel);
		else
			sendResponse(username, channel, "To start a game please type only '!start'");
		break;
	case CMD_FIRE:
		target = message.substr(message.find_last_of(" ") + 1);
		if (message.length() == e_fire)
			fireShot(username, channel, target);
		else
			sendResponse(username, channel, "This is not a valid battleships target: '" + capitalize(target) + "'");
		break;
	default:
		break;
	}
}






/* ===================== Bot Commands ===================== */

void	Bot::startGame( const std::string& username, const std::string& channel )
{
	Game*	game;

	try
	{
		auto it = m_games.find(username);

		if ( it == m_games.end() )
		{
			game = new Game {};
			m_games.insert({username, game});
		}
		else
			game = it->second;
	}
	catch ( const std::exception& e )
	{
		sendResponse(username, channel, e.what());
		return;
	}

	#ifdef DEBUG
	sendGrid(username, channel, "CHEAT GRID for " + username + "'s game", game->getGridObject());
	#endif

	sendGrid(username, channel, "Battleships grid for " + username + "'s game", game->getPlayerGridObject());
}

void	Bot::fireShot( const std::string& username, const std::string& channel, const std::string& target )
{
	auto it = m_games.find(username);
	if (it == m_games.end())
	{
		sendResponse(username, channel, "You need to start a game before you can fire a shot. You can start a new game by typing '!start'");
		return;
	}

	Game* game { it->second };

	if (!game->validInput(target))
	{
		sendResponse(username, channel, target + " is not a valid target!");
		return;
	}

	ShotResult sr { game->processShot(target) };

	sendGrid(username, channel, "Battleships grid for " + username + "'s game", game->getPlayerGridObject());

	switch (sr)
	{
	case ShotResult::MISS:
		sendFeedback(username, channel, "Your shot at " + capitalize(target) + " missed!");
		break;
	case ShotResult::HIT:
		sendFeedback(username, channel, "You hit an enemy ship at " + capitalize(target) + "!");
		break;
	case ShotResult::SUNK:
		sendFeedback(username, channel, "You sunk an enemy ship! Congrats, keep going!");
		break;
	case ShotResult::WON:
		sendFeedback(username, channel, "Well done, you sunk all of the enemy's ships!");
		m_games.erase(username);
		sendFeedback(username, channel, "To play again just type !start in the Battleships channel or as a private message to BattleShipsBot.");
		break;
	default:
		break;
	}
}





/* ===================== Bot Response ===================== */

void	Bot::sendGrid( const std::string& username, const std::string& channel, const std::string& header, const Grid& grid ) const
{
	std::string 		gridMsg { grid.getGridMsg() };
	std::istringstream	iss { gridMsg };
	std::string			line {};

	sendResponse(username, channel, "");
	sendResponse(username, channel, header);
	sendResponse(username, channel, "");
	while ( std::getline(iss, line) )
		sendResponse(username, channel, line);
}

void	Bot::sendFeedback( const std::string& username, const std::string& channel, const std::string& msg ) const
{
	sendResponse(username, channel, "");
	sendResponse(username, channel, msg);
}

void	Bot::sendResponse( const std::string& username, const std::string& channel, const std::string& msg ) const
{
	std::string	prefix {};
	std::string	response {};

	if (channel.length())
		prefix = "PRIVMSG " +  channel + " :";
	else
		prefix = "PRIVMSG " +  username + " :";

	response = prefix + msg + "\r\n";

	send(m_bot_socket_fd, response.data(), response.length(), 0);
}





/* ===================== Parsing ===================== */

std::string	Bot::getUserName( const std::string& buffer ) const
{
	return ( buffer.substr(1, buffer.find_first_of('!') - 1) );
}

std::string Bot::getMessage( const std::string& buffer ) const
{
	std::string message { buffer.substr(buffer.find(" :", 2) + 2) };

	return ( rtrim(message) );
}

std::string Bot::getChannelName( const std::string& buffer ) const
{
	std::size_t start_idx {};
	std::size_t end_idx {};

	if ((start_idx = buffer.find_first_of("#")) == std::string::npos)
		return "";

	end_idx = buffer.substr(start_idx).find_first_of(" ");

	return ( buffer.substr(start_idx, start_idx + end_idx - start_idx + 1) );
}

Bot::CommandType Bot::getCmdType( const std::string& command ) const
{
	auto it { k_commands.find(command) };

	return ( it != k_commands.end() ? it->second : CMD_UNKNOWN );
}





/* ===================== Utility ===================== */

std::string&	Bot::rtrim( std::string& s ) const
{
	std::size_t end_idx { s.find_last_not_of(" \t\n\r") };

	if (end_idx != std::string::npos)
		s.erase(end_idx + 1);
	else
		s.clear();

	return s;
}

std::string	Bot::capitalize( const std::string& target ) const
{
	std::string capTarget {};

	capTarget = target;
	capTarget[0] = std::toupper(capTarget[0]);

	return capTarget;
}
