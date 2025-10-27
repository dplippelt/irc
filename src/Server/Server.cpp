/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 13:10:45 by dlippelt          #+#    #+#             */
/*   Updated: 2025/10/27 16:24:13 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/Server.hpp"

/* ==================== Constructors & Destructors ==================== */

Server::~Server()
{
	if (close(m_listening_socket_fd) == -1)
		std::cerr << "Error: failed to close socket file descriptor\n";
}

Server::Server( const std::string& port, std::string_view pw )
	: m_pw { pw }
{
	setPort(port);

	m_listening_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_listening_socket_fd == -1)
	{
		throw std::runtime_error("Error: failed to create socket\n");
	}

	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons(m_port);
	m_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(m_listening_socket_fd, reinterpret_cast<struct sockaddr *>(&m_addr), sizeof(m_addr)) == -1)
		throw std::runtime_error("Error: failed to bind address to socket\n");

	if (listen(m_listening_socket_fd, LISTEN_BACKLOG) == -1)
		throw std::runtime_error("Error: failed to set socket as a passive socket listening for incoming connections\n");
}

Server::Server( const Server& ) = default;

Server& Server::operator=( const Server& ) = default;


/* ==================== Utility ==================== */

void Server::setPort( const std::string& port )
{
	for ( char c : port )
	{
		if (!std::isdigit(static_cast<unsigned char>(c)))
			throw std::runtime_error("Error: port '" + port + "' contains non-digit characters\n");
	}

	try
	{
		m_port = std::stoi(port);
	}
	catch(const std::exception& e)
	{
		throw std::runtime_error("Error: could not convert port '" + port + "' to integer\n");
	}

	if (m_port < 1 || m_port > 65535)
		throw std::runtime_error("Error: port '" + port + "' must be between 1 and 65535\n");
}
