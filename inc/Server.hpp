/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: dlippelt <dlippelt@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/27 13:04:53 by dlippelt      #+#    #+#                 */
/*   Updated: 2025/11/05 11:33:18 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <sstream>
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
#include "User.hpp"
#include "Channel.hpp"
#include "Commands.hpp"

// #define DEBUG

class Channel;
class Commands;

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
		static const int									s_listen_backlog { 50 };
		static inline const std::string						s_server_name { "ft_irc" };
		static inline const std::string						s_server_version { "0.1" };
		static inline const std::string						s_user_modes { "o" };
		static inline const std::string						s_channel_modes { "itkol" };
		static inline const std::vector<std::string_view>	s_commands { "PING", "NICK", "USER", "PASS", "MODE", "WHOIS", "JOIN", "PART", "KICK", "NO_CMD" };

		std::string							m_pw {};
		int									m_listening_socket_fd {};
		struct addrinfo						*m_addr {};
		std::map<int, User*>				m_users {};
		std::map<std::string, Channel*>		m_channels {};
		Commands*							m_commands {};
		std::vector<struct pollfd>			m_pollfds {};

		void		validatePort( const std::string& port );
		void		acceptConn();
		void		processClientAct( int client_fd );
		void		removeClient( int client_fd );
		void		processBuffer( const std::string& buffer, ssize_t bytes, int client_fd );
		bool		foundEndOfMessage( std::string_view buffer, std::size_t *start_idx, std::size_t *eom_idx );
		void		processMsg( std::string_view buffer, std::size_t start_idx, std::size_t end_idx, int client_fd );
		// std::string	getNumericReply( int i, const std::string& nick, const std::string& user, const std::string& host );
		bool		userIsAuthenticated( int client_fd );
		// void		userAuthentication( int client_fd );

		void	pong( std::vector<std::string>& cmd_params, int client_fd );
};

enum Command
{
	PING,
	NICK,
	USER,
	PASS,
	MODE,
	WHOIS,
	JOIN,
	PART,
	KICK,
	NO_CMD
};
