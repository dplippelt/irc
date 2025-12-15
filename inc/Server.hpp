/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 13:04:53 by dlippelt          #+#    #+#             */
/*   Updated: 2025/12/15 12:56:43 by dlippelt         ###   ########.fr       */
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
#include <signal.h>
#include "User.hpp"
#include "Channel.hpp"
#include "Commands.hpp"
#include "Parser.hpp"
#include "ResponseHandler.hpp"

extern volatile sig_atomic_t g_quit;

class Channel;

class Server
{
	public:
		~Server();
		Server() = delete;
		Server( const std::string& port, std::string_view pw );
		Server( const Server& );
		Server& operator=( const Server& );

		static void sigHandler(int signum);

		void	doPoll();
		void	removeClient( int client_fd, const std::string& quitMessage = "" );

		std::map<int, User*>&					getUsers();
		std::map<std::string, Channel*>&		getChannels();
		const std::map<int, User*>&				getUsers() const;
		const std::map<std::string, Channel*>&	getChannels() const;
		const std::string& 						getPassword() const { return m_pw; }

		void	sendToClient(int fd, const std::string& message);

		#ifdef DEBUG
		void	printModeStates() const;
		#endif

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
		std::vector<struct pollfd>			m_pollfds {};
		std::map<int, Parser>				m_messagesList{};
		ResponseHandler*					m_responseHandler;

		void		validatePort( const std::string& port );
		void		acceptConn();
		void		processClientAct( int client_fd );
		void		processBuffer( const std::string& buffer, int client_fd );
		void		enablePollOut(int fd);
		void		disablePollOut(int fd);
		void		trySendPendingData(int client_fd);
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
