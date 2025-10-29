/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 13:10:45 by dlippelt          #+#    #+#             */
/*   Updated: 2025/10/29 11:45:56 by dlippelt         ###   ########.fr       */
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
	, m_hints { AI_PASSIVE, AF_INET, SOCK_STREAM, 0, 0, NULL, NULL, NULL }
{

	validatePort(port);

	if ( getaddrinfo(NULL, port.data(), &m_hints, &m_addr) )
		throw std::runtime_error("Error: failed to get required address info");
	if ( m_addr->ai_next != NULL )
		std::cerr << "Warning: found more than one matching set of address info";

	m_listening_socket_fd = socket(m_addr->ai_family, m_addr->ai_socktype, m_addr->ai_protocol);
	if ( m_listening_socket_fd == -1 )
		throw std::runtime_error("Error: failed to create socket");
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

Server::Server( const Server& ) = default;

Server& Server::operator=( const Server& ) = default;





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
