/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 13:10:45 by dlippelt          #+#    #+#             */
/*   Updated: 2025/10/28 09:26:23 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/Server.hpp"

/* ==================== Constructors & Destructors ==================== */

Server::~Server()
{
	for ( auto pollfd : m_pollfds )
	{
		if ( close(pollfd.fd) == -1 )
			std::cerr << "Warning: failed to close socket file descriptor '" << pollfd.fd << "'" << std::endl;
	}
}

Server::Server( const std::string& port, std::string_view pw )
	: m_pw { pw }
{
	setPort(port);

	m_listening_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if ( m_listening_socket_fd == -1 )
		throw std::runtime_error("Error: failed to create socket");
	if ( fcntl(m_listening_socket_fd, F_SETFL, O_NONBLOCK) == -1 )
		throw std::runtime_error("Error: failed to set listening socket to non-blocking");
	m_pollfds.push_back( {m_listening_socket_fd, POLLIN, 0} );

	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons(m_port);
	m_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if ( bind(m_listening_socket_fd, reinterpret_cast<struct sockaddr *>(&m_addr), sizeof(m_addr)) == -1 )
		throw std::runtime_error("Error: failed to bind address to socket");

	if ( listen(m_listening_socket_fd, LISTEN_BACKLOG) == -1 )
		throw std::runtime_error("Error: failed to set socket as a passive socket listening for incoming connections");
}

Server::Server( const Server& ) = default;

Server& Server::operator=( const Server& ) = default;





/* ==================== Helpers ==================== */

void	Server::setPort( const std::string& port )
{
	for ( char c : port )
	{
		if ( !std::isdigit(static_cast<unsigned char>(c)) )
			throw std::runtime_error("Error: port '" + port + "' contains non-digit characters");
	}

	try
	{
		m_port = std::stoi(port);
	}
	catch ( const std::exception& e )
	{
		throw std::runtime_error("Error: could not convert port '" + port + "' to integer");
	}

	if ( m_port < 1 || m_port > 65535 )
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

	m_client_addrss.push_back(client_addr);		// might not need this vector(?)
	m_client_socket_fds.push_back(client_fd);	// might not need this vector, m_pollfds already stores the same info
	m_pollfds.push_back( {client_fd, POLLIN, 0} );

	#ifdef DEBUG
	std::cout << "Accepted client connection" << std::endl;
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
			if ( m_pollfds[i].fd != m_listening_socket_fd )
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
	std::cout << "Client disconnected" << std::endl;
	#endif

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

//THIS IS A TEMPORARY PLACEHOLDER FUNCTION THAT JUST ECHOES THE CLIENT ACTIVITY TO THE TERMINAL
void	Server::processClientAct( int client_fd )
{
	#ifdef DEBUG
	std::cout << "Activity detected on client socket!" << std::endl;
	#endif

	char buffer[512];
	ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

	if ( bytes <= 0 )
	{
		removeClient(client_fd);
		return ;
	}

	buffer[bytes] = '\0';
	std::cout << "Client acitivity:\n" << buffer;
}
