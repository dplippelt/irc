/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 13:10:45 by dlippelt          #+#    #+#             */
/*   Updated: 2025/10/27 13:33:38 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

/* ==================== Constructors & Destructors ==================== */

Server::~Server() = default;

Server::Server( const std::string& port, std::string_view pw )
	: m_pw { pw }
{
	try
	{
		m_port = std::stoi(port);
	}
	catch(const std::exception& e)
	{
		throw std::runtime_error("Error: could not convert port parameter '" + port + "' to integer because: " + e.what());
	}

	m_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket_fd == -1)
	{
		throw std::runtime_error("Error: failed to create socket");
	}
}

Server::Server( const Server& ) = default;

Server& Server::operator=( const Server& ) = default;
