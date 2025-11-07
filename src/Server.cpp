/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmitsuya <tmitsuya@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 13:10:45 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/07 14:39:09 by tmitsuya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

/* ==================== Constructors & Destructors ==================== */

Server::~Server()
{
	delete m_commands;

	for ( std::map<int, User*>::iterator it = m_users.begin(); it != m_users.end(); ++it )
		delete it->second;
	for ( std::map<std::string, Channel*>::iterator it = m_channels.begin(); it != m_channels.end(); ++it )
		delete it->second;
	if ( m_addr != nullptr )
		freeaddrinfo(m_addr);
	for ( auto pollfd : m_pollfds )
	{
		if ( close(pollfd.fd) == -1 )
			std::cerr << "Warning: failed to close socket file descriptor '" << pollfd.fd << "'" << std::endl;
	}
}

Server::Server( const std::string& port, std::string_view pw )
	: m_pw { pw }
{
	struct addrinfo	hints { AI_PASSIVE, AF_INET, SOCK_STREAM, 0, 0, NULL, NULL, NULL };

	validatePort(port);

	if ( getaddrinfo(NULL, port.data(), &hints, &m_addr) )
		throw std::runtime_error("Error: failed to get required address info");
	if ( m_addr->ai_next != NULL )
		std::cerr << "Warning: found more than one matching set of address info";

	m_listening_socket_fd = socket(m_addr->ai_family, m_addr->ai_socktype, m_addr->ai_protocol);
	if ( m_listening_socket_fd == -1 )
		throw std::runtime_error("Error: failed to create socket");
	int	opt {1};
	if ( setsockopt(m_listening_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		throw std::runtime_error("Error: failed to set socket option SO_REUSEADDR");
	if ( fcntl(m_listening_socket_fd, F_SETFL, O_NONBLOCK) == -1 )
		throw std::runtime_error("Error: failed to set listening socket to non-blocking");
	m_pollfds.push_back( {m_listening_socket_fd, POLLIN, 0} );

	if ( bind(m_listening_socket_fd, m_addr->ai_addr, m_addr->ai_addrlen) == -1 )
		throw std::runtime_error("Error: failed to bind address to socket");
	freeaddrinfo(m_addr);
	m_addr = nullptr;

	if ( listen(m_listening_socket_fd, s_listen_backlog) == -1 )
		throw std::runtime_error("Error: failed to set socket as a passive socket listening for incoming connections");

	m_commands = new Commands(m_users, m_channels, m_pw);
}

/* ==================== Initial Server Setup ==================== */

void	Server::validatePort( const std::string& port )
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

void	Server::acceptConn()
{
	#ifdef DEBUG
	std::cout << "Activity detected on listening socket!" << std::endl;
	#endif

	struct sockaddr_in	client_addr {};
	socklen_t			client_addr_len { sizeof(client_addr) };

	int client_fd { accept(m_listening_socket_fd, reinterpret_cast<struct sockaddr *>(&client_addr), &client_addr_len) };
	if ( client_fd == -1 )
		throw std::runtime_error("Error: failed to accept incoming connection");
	if ( fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1 )
		throw std::runtime_error("Error: failed to set client socket to non-blocking");

	User* newUser = new User(client_fd);
	m_users.insert( { client_fd, newUser } );
	m_pollfds.push_back( {client_fd, POLLIN, 0} );

	#ifdef DEBUG
	std::cout << "Accepted client connection (client fd: " << client_fd << ")" << std::endl;
	#endif
}

void	Server::doPoll()
{
	#ifdef DEBUG
	std::cout << "Polling..." << std::endl;
	#endif

	int ret = poll(m_pollfds.data(), m_pollfds.size(), -1);

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

	for ( int i {static_cast<int>(m_pollfds.size()) - 1}; i >= 0; --i )
	{
		if ( m_pollfds[i].revents & (POLLHUP | POLLERR) )
		{
			if ( m_pollfds[i].fd == m_listening_socket_fd )
				throw std::runtime_error("Error: error on listening socket");
			else
			{
				removeClient(m_pollfds[i].fd);
				continue;
			}
		}

		if ( m_pollfds[i].revents & POLLIN )
		{
			if ( m_pollfds[i].fd == m_listening_socket_fd )
				acceptConn();
			else
				processClientAct(m_pollfds[i].fd);
		}
	}
}

void	Server::removeClient( int client_fd )
{
	#ifdef DEBUG
	std::cout << "Client disconnected (client fd: " << client_fd << ")" << std::endl;
	#endif

	std::map<int, User*>::iterator it = m_users.find(client_fd);
	if ( it != m_users.end() )
	{
		delete it->second;
		m_users.erase(it);
	}

	if ( close(client_fd) == -1 )
		std::cerr << "Warning: failed to close client file descriptor" << std::endl;

	for ( auto it {m_pollfds.begin()}; it != m_pollfds.end(); ++it )
	{
		if ( (*it).fd == client_fd )
		{
			m_pollfds.erase(it);
			break;
		}
	}
}

void	Server::processClientAct( int client_fd )
{
	#ifdef DEBUG
	std::cout << "Activity detected on client socket!" << std::endl;
	#endif

	char buffer[512];
	ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

	if ( bytes == 0 )
	{
		removeClient(client_fd);
		return;
	}
	if ( bytes == -1 )
	{
		if ( errno == EAGAIN || errno == EWOULDBLOCK )
			return;
		std::cerr << "Error while receiving message from client. Removing client..." << std::endl;
		removeClient(client_fd);
		return;
	}

	buffer[bytes] = '\0';

	m_massagesList.emplace(client_fd, Parser{}); // [Takato]: added
	// processBuffer(buffer, bytes, client_fd);  // [Takato]: commented out
}

Server::Server( const Server& ) = default;

Server& Server::operator=( const Server& ) = default;

/* ==================== Message Processing ==================== */

void	Server::processBuffer( const std::string& buffer, ssize_t bytes, int client_fd )
{
	static std::map<int, std::string>	remainders {};
	std::string&						remainder { remainders[client_fd] };
	std::size_t							eom_idx {};
	std::size_t							start_idx {};
	std::size_t							start_idx_prev {};

	while ( eom_idx != std::string::npos )
	{
		start_idx_prev = start_idx;
		if ( !foundEndOfMessage(buffer, &start_idx, &eom_idx) )
			break;

		processMsg(remainder + buffer, start_idx_prev, start_idx + remainder.length(), client_fd);
		remainder.clear();
	}
	if ( start_idx != static_cast<std::size_t>(bytes) )
		remainder += buffer.substr(start_idx);
}

bool	Server::foundEndOfMessage( std::string_view buffer, std::size_t *start_idx, std::size_t *eom_idx )
{
	*eom_idx = buffer.find("\r\n", *start_idx);

	if ( *eom_idx != std::string::npos )
	{
		*start_idx = *eom_idx + 2;
		return (true);
	}
	return (false);
}

void	Server::processMsg( std::string_view buffer, std::size_t start_idx, std::size_t end_idx, int client_fd )
{
	std::string					msg { buffer.substr(start_idx, end_idx - start_idx) };
	std::string					el {};
	std::istringstream			iss { msg };
	std::vector<std::string>	cmd_params {};
	std::string					command {};
	bool						first_param { true };

	#ifdef DEBUG
	std::cout << "Processing message: " << msg << std::endl;
	#endif

	std::map<int, User*>::iterator userIt = m_users.find(client_fd);
	if (userIt == m_users.end())
	{
		std::cerr << "Error: User not found for fd " << client_fd << std::endl;
		return;
	}
	User* user = userIt->second;

	while ( iss >> el )
	{
		if ( first_param )
		{
			command = el;
			first_param = false;
			continue;
		}

		if ( el[0] == ':' )
		{
			std::string rest;
			std::getline(iss, rest);
			el += rest;
			cmd_params.push_back(el);
			break;
		}
		cmd_params.push_back(el);
	}

	if (command == "PING")
	{
		pong(cmd_params, client_fd);
		return;
	}

	m_commands->executeCommand(user, command, cmd_params);

	std::cout << msg;
}

/* ==================== (Mock) Authentication ==================== */

bool	Server::userIsAuthenticated( int client_fd )
{
	std::map<int, User*>::iterator it = m_users.find(client_fd);
	if (it != m_users.end())
		return it->second->isAuthenticated();
	return false;
}

/* ==================== Pong implementation so connection doesn't time out ==================== */

void	Server::pong( std::vector<std::string>& cmd_params, int client_fd )
{
	std::string pong_str { "PONG :" };

	if (!cmd_params.empty())
		pong_str.append(cmd_params[0]).append("\r\n");
	else
		pong_str.append("ft_irc\r\n");

	send(client_fd, pong_str.data(), pong_str.length(), 0);

	#ifdef DEBUG
	std::cout << "Sent PONG response to client fd " << client_fd << std::endl;
	#endif
}
