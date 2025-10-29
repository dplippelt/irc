/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 13:04:53 by dlippelt          #+#    #+#             */
/*   Updated: 2025/10/29 11:38:18 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <netdb.h>

#define LISTEN_BACKLOG 50
#define DEBUG

class Server
{
	public:
		~Server();
		Server() = delete;
		Server( const std::string& port, std::string_view pw );
		Server( const Server& );
		Server& operator=( const Server& );

		void	doPoll();

	private:
		std::string							m_pw {};
		int									m_listening_socket_fd {};
		struct addrinfo						m_hints {};
		struct addrinfo						*m_addr {};
		std::map<int, struct sockaddr_in>	m_client_addrss {};
		std::vector<struct pollfd>			m_pollfds {};

		void	validatePort( const std::string& port );
		void	acceptConn();
		void	processClientAct(int client_fd);
		void	removeClient(int client_fd);
};
