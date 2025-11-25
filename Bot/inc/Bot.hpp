/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 10:37:54 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/25 15:54:16 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <string_view>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <poll.h>

#include "Game.hpp"

class Bot
{
	public:
		~Bot();
		Bot() = delete;
		Bot( const std::string& server_address, const std::string& server_port, std::string_view pw );
		Bot( const Bot& ) = delete;
		Bot& operator=( const Bot& ) = delete;

		void	doPoll();

	private:

		enum CommandLengths
		{
			e_start = 6,
			e_fire = 8,
		};

		enum CommandType
		{
			CMD_START,
			CMD_FIRE,
			CMD_UNKNOWN
		};

		static inline const std::map<std::string, CommandType> k_commands
		{
			{"!start", CMD_START},
			{"!fire", CMD_START}
		};

		std::string			m_pw {};
		struct addrinfo*	m_server_addr {};
		int					m_bot_socket_fd {};
		struct pollfd		m_pollfd {};

		std::map<std::string, Game*>	m_games {};
		std::string 					m_prefix {};

		void	validatePort( const std::string& port ) const;
		void	validateAddress( const std::string& address) const;
		void	authenticateAndJoin() const;
		void	receiveMessage();
		void	processBuffer( const std::string& buffer );

		void	startGame( const std::string& username, const std::string& channel );
		void	fireShot( const std::string& username, const std::string& channel,const std::string& target );

		void	sendGrid( const std::string& username, const std::string& channel, const std::string& header, const Grid& grid ) const;
		void	sendFeedback( const std::string& username, const std::string& channel, const std::string& msg ) const;
		void	sendResponse( const std::string& username, const std::string& channel, const std::string& msg ) const;

		std::string	getUserName( const std::string& buffer ) const;
		std::string getMessage( const std::string& buffer ) const;
		std::string getChannelName( const std::string& buffer ) const;
		CommandType getCmdType( const std::string& command ) const;

		std::string&	rtrim( std::string& s ) const;
		std::string		capitalize( const std::string& target ) const;
};
