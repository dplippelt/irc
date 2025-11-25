/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 10:39:01 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/20 18:43:02 by dlippelt         ###   ########.fr       */
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

void	Bot::processBuffer( const std::string& buffer )
{
	std::string username { getUserName(buffer) };
	std::string message { getMessage(buffer) };
	std::string channel { getChannelName(buffer) };

	#ifdef DEBUG
	std::cout << "Sender: " << username << std::endl;
	std::cout << "Channel: " << channel << std::endl;
	std::cout << "Message: " << message << std::endl;
	#endif

	if (message.substr(0, 6) == "!start")
		startGame(username, channel);
	// if (message.substr(0, 5) == "!fire")
	// 	fireShot(username)
}

std::string	Bot::getUserName( const std::string& buffer ) const
{
	return ( buffer.substr(1, buffer.find_first_of('!') - 1) );
}

std::string Bot::getMessage( const std::string& buffer ) const
{
	return ( buffer.substr(buffer.find(" :", 2) + 2) );
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

void	Bot::startGame( const std::string& username, const std::string& channel )
{
	std::string response {};
	std::string prefix {};
	Game*		game;

	if (channel.length())
		prefix = "PRIVMSG " +  channel + " :";
	else
		prefix = "PRIVMSG " +  username + " :";

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
		response = prefix + e.what() + "\r\n";
		send(m_bot_socket_fd, response.data(), response.length(), 0);
		return;
	}

	std::string 		gridMsg { game->getGridObject().getGridMsg() };
	std::istringstream	iss { gridMsg };
	std::string			line {};

	while ( std::getline(iss, line) )
	{
		response = prefix + line + "\r\n";
		send(m_bot_socket_fd, response.data(), response.length(), 0);
	}
}
