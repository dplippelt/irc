/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 10:37:54 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/25 13:34:37 by dlippelt         ###   ########.fr       */
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

		void	sendResponse( const std::string& username, const std::string& channel, const std::string& msg );

		std::string	getUserName( const std::string& buffer ) const;
		std::string getMessage( const std::string& buffer ) const;
		std::string getChannelName( const std::string& buffer ) const;

		std::string&	rtrim( std::string& s ) const;

};
