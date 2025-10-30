/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 13:10:45 by dlippelt          #+#    #+#             */
/*   Updated: 2025/10/30 12:11:29 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/Server.hpp"


/* ==================== Constructors & Destructors ==================== */

Server::~Server()
{
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

	validatePort(port);

	if ( getaddrinfo(NULL, port.data(), &m_hints, &m_addr) )
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

	if ( listen(m_listening_socket_fd, LISTEN_BACKLOG) == -1 )
		throw std::runtime_error("Error: failed to set socket as a passive socket listening for incoming connections");
}





/* ==================== Helpers ==================== */

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

std::string	Server::getNumericReply( int i, const std::string& nick, const std::string& user, const std::string& host )
{
	switch (i)
	{
	case 1:
		return (":" + SERVER_NAME + " 001 " + nick + " :Welcome to our IRC Network " + nick + "!" + user + "@" + host + "\r\n");
	case 2:
		return (":" + SERVER_NAME + " 002 " + nick + " :Your host is " + SERVER_NAME + ", running version " + SERVER_VERSION + "\r\n");
	case 3:
		return (":" + SERVER_NAME + " 003 " + nick + " :This server was created as part of the Codam project ft_irc." + "\r\n");
	case 4:
		return (":" + SERVER_NAME + " 004 " + nick + " " + SERVER_NAME + " " + SERVER_VERSION + " " + USER_MODES + " " + CHANNEL_MODES + "\r\n");
	default:
		return ("");
	}
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

	m_user_auth_status.insert( {client_fd, false} );
	m_client_addrss.insert( {client_fd, client_addr} );
	m_pollfds.push_back( {client_fd, POLLIN, 0} );

	#ifdef DEBUG
	std::cout << "Accepted client connection (client port: " << client_addr.sin_port << ")" << std::endl;
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
			return ;
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
				continue ;
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
	std::cout << "Client disconnected (client port: " << m_client_addrss.find(client_fd)->second.sin_port << ")" << std::endl;
	#endif

	m_client_addrss.erase(client_fd);

	if ( close(client_fd) == -1 )
		std::cerr << "Warning: failed to close client file descriptor" << std::endl;

	for ( auto it {m_pollfds.begin()}; it != m_pollfds.end(); ++it )
	{
		if ( (*it).fd == client_fd )
		{
			m_pollfds.erase(it);
			break ;
		}
	}
}

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
			break ;

		printMsg(remainder + buffer, start_idx_prev, start_idx + remainder.length());
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

void	Server::printMsg( std::string_view buffer, std::size_t start_idx, std::size_t end_idx )
{
	std::cout << buffer.substr(start_idx, end_idx - start_idx);
}

bool	Server::userIsAuthenticated( int client_fd )
{
	return ( m_user_auth_status.find(client_fd)->second );
}

void	Server::userAuthentication( int client_fd )
{
	//check conditions for user to be authentiacted, this is temporary placeholder check
	if (m_user_auth_status.find(client_fd)->second == false)
	{
		//if check passes send numeric replies to client to confirm connection and auth status to ok/true
		std::string	numericReply;
		for ( int i {1}; i < 5; ++i )
		{
			numericReply = getNumericReply(i, "testNick", "testUser", "localhost");
			if ( send(client_fd, numericReply.data(), numericReply.length(), 0) == -1 )
				std::cerr << "Warning: failed to send numeric reply to client" << std::endl;
		}
		m_user_auth_status.find(client_fd)->second = true;
	}
	//else wait to receive more info
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
		return ;
	}
	if ( bytes == -1 )
	{
		if ( errno == EAGAIN || errno == EWOULDBLOCK )
			return ;
		std::cerr << "Error while receiving message from client. Removing client..." << std::endl;
		removeClient(client_fd);
		return ;
	}

	buffer[bytes] = '\0';
	processBuffer(buffer, bytes, client_fd);

	if ( !userIsAuthenticated(client_fd) )
		userAuthentication(client_fd);
}
