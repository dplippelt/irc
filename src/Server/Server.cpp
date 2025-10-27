/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 13:10:45 by dlippelt          #+#    #+#             */
/*   Updated: 2025/10/27 14:08:43 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/Server.hpp"

/* ==================== Constructors & Destructors ==================== */

Server::~Server() = default;

Server::Server( const std::string& port, std::string_view pw )
	: m_pw { pw }
{
	setPort(port);

	m_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket_fd == -1)
	{
		throw std::runtime_error("Error: failed to create socket\n");
	}
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
