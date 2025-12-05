/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 10:39:01 by dlippelt          #+#    #+#             */
/*   Updated: 2025/12/05 13:11:31 by dlippelt         ###   ########.fr       */
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

Bot::Bot( const std::string& server_port, std::string_view pw )
	: m_pw { pw }
{
	struct addrinfo	hints { 0, AF_INET, SOCK_STREAM, 0, 0, NULL, NULL, NULL };

	validatePort(server_port);

	if (getaddrinfo("localhost", server_port.data(), &hints, &m_server_addr))
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





/* ==================== Initial Bot Setup ==================== */

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
	std::string irc_cmd { getIRCCommand(buffer) };

	#ifdef DEBUG
	std::cout << "Sender: " << username << std::endl;
	std::cout << "Channel: " << channel << std::endl;
	std::cout << "Message: " << message << std::endl;
	std::cout << "IRC Command: " << irc_cmd << std::endl;
	std::cout << "Message length: " << message.length() << std::endl;
	#endif

	if ( needWelcome(irc_cmd, username) )
		BotResponseHandler::sendWelcome(m_bot_socket_fd, username, channel);

	trackChannelMembers(username, irc_cmd);

	BotCommands::executeCommand(username, channel, message, *this);
}

bool	Bot::needWelcome( const std::string& irc_cmd, const std::string& username )
{
	if ( username == "BattleshipsBot" )
		return false;

	if ( irc_cmd != "JOIN" && irc_cmd != "PRIVMSG" )
		return false;

	for ( auto it { m_welcomed.begin() }; it != m_welcomed.end(); ++it )
		if ( (*it) == username )
			return false;

	m_welcomed.push_back(username);
	return true;
}





/* ===================== Channel Member Tracking ===================== */

void	Bot::trackChannelMembers( const std::string& username, const std::string& irc_cmd )
{
	switch ( getIRCCmdType(irc_cmd) )
	{
	case CMD_JOIN:
		addChannelMember(username);
		break;
	case CMD_PART:
		removeChannelMember(username);
		break;
	case CMD_QUIT:
		removeChannelMember(username);
		break;
	case CMD_KICK:
		removeChannelMember(username);
		break;
	default:
		break;
	}
}

void	Bot::addChannelMember( const std::string& username )
{
	m_channel_members.push_back(username);
}

void	Bot::removeChannelMember( const std::string& username )
{
	for ( auto it { m_channel_members.begin() }; it != m_channel_members.end(); ++it )
	{
		if ( (*it) == username )
		{
			m_channel_members.erase(it);
			return;
		}
	}
}

bool	Bot::memberInChannel( const std::string& username ) const
{
	for ( auto member : m_channel_members )
		if ( member == username )
			return true;

	return false;
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
	std::string channelName {};

	start_idx = buffer.find_first_of("#");
	if ( start_idx == std::string::npos )
		return "";

	end_idx = buffer.find_first_of(" \r\n", start_idx);

	channelName = buffer.substr(start_idx, end_idx - start_idx);

	return ( rtrim(channelName) );
}

std::string Bot::getIRCCommand( const std::string& buffer ) const
{
	std::size_t start_idx {};
	std::size_t end_idx {};

	start_idx = buffer.find_first_of(" ") + 1;
	end_idx = buffer.find_first_of(" ", start_idx );

	return ( buffer.substr(start_idx, end_idx - start_idx) );
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

Bot::BotIRCCommandType Bot::getIRCCmdType( const std::string& irc_cmd ) const
{
	auto it { k_commands.find(irc_cmd) };

	return ( it != k_commands.end() ? it->second : CMD_UNKNOWN );
}





/* ===================== Getters and Setters ===================== */

int	Bot::getSocket() const
{
	return m_bot_socket_fd;
}

const std::map<std::string, Game*>&	Bot::getGames() const
{
	return m_games;
}

void	Bot::removeGame( std::string username )
{
	m_games.erase(username);
}

void	Bot::addGame( std::string username, Game* game )
{
	m_games.insert( {username, game} );
}

const std::map<std::pair<std::string, std::string>, MPGame*>&	Bot::getMPGames() const
{
	return m_mp_games;
}

std::map<std::pair<std::string, std::string>, MPGame *>::iterator	Bot::getMPGame( const std::string& player_one, const std::string& player_two )
{
	auto it { m_mp_games.find({ player_one, player_two }) };
	if (it != m_mp_games.end())
		return it;

	it = m_mp_games.find({ player_two, player_one });
	return it;
}

void	Bot::removeMPGame( const std::string& player_one, const std::string& player_two )
{
	m_mp_games.erase( {player_one, player_two} );
	m_mp_games.erase( {player_two, player_one} );
}

void	Bot::addMPGame( std::pair<std::string, std::string> usernames, MPGame* mp_game )
{
	m_mp_games.insert( {usernames, mp_game} );
}

const std::vector<std::pair<std::string, std::string>>&	Bot::getChallenges() const
{
	return m_challenges;
}

void	Bot::addChallenge( std::string player_one, std::string player_two )
{
	m_challenges.push_back( {player_one, player_two} );
}

void	Bot::removeChallenge( std::string player_one, std::string player_two )
{
	for ( auto it {m_challenges.begin()}; it != m_challenges.end(); ++it )
	{
		if ( (it->first == player_one && it->second == player_two) || (it->first == player_two && it->second == player_one) )
		{
			m_challenges.erase(it);
			return;
		}
	}
}
