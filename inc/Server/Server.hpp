/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 13:04:53 by dlippelt          #+#    #+#             */
/*   Updated: 2025/10/30 12:07:42 by dlippelt         ###   ########.fr       */
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

#define DEBUG

class Server
{
	public:
		~Server();
		Server() = delete;
		Server( const std::string& port, std::string_view pw );
		Server( const Server& ) = delete;
		Server& operator=( const Server& ) = delete;

		void	doPoll();

	private:
		const int			LISTEN_BACKLOG { 50 };
		const std::string	SERVER_NAME { "ft_irc" };
		const std::string	SERVER_VERSION { "0.1" };
		const std::string	USER_MODES { "o" };
		const std::string	CHANNEL_MODES { "itkol" };

		std::string							m_pw {};
		int									m_listening_socket_fd {};
		struct addrinfo						m_hints { AI_PASSIVE, AF_INET, SOCK_STREAM, 0, 0, NULL, NULL, NULL };
		struct addrinfo						*m_addr {};
		std::map<int, struct sockaddr_in>	m_client_addrss {};
		std::vector<struct pollfd>			m_pollfds {};

		std::map<int, bool>					m_user_auth_status {};


		void		validatePort( const std::string& port );
		void		acceptConn();
		void		processClientAct( int client_fd );
		void		removeClient( int client_fd );
		void		processBuffer( const std::string& buffer, ssize_t bytes, int client_fd );
		bool		foundEndOfMessage( std::string_view buffer, std::size_t *start_idx, std::size_t *eom_idx );
		void		printMsg( std::string_view buffer, std::size_t start_idx, std::size_t end_idx );
		std::string	getNumericReply( int i, const std::string& nick, const std::string& user, const std::string& host );
		bool		userIsAuthenticated( int client_fd );
		void		userAuthentication( int client_fd );
};
