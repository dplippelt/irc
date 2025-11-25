/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 10:37:54 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/25 17:59:36 by dlippelt         ###   ########.fr       */
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
#include "BotCommands.hpp"

class Bot
{
	public:
		~Bot();
		Bot() = delete;
		Bot( const std::string& server_address, const std::string& server_port, std::string_view pw );
		Bot( const Bot& ) = delete;
		Bot& operator=( const Bot& ) = delete;

		void	doPoll();

		int									getSocket() const;
		const std::map<std::string, Game*>	getGames() const;
		void								removeGame( std::string username );
		void								addGame( std::string username, Game* game );

	private:
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

		std::string	getUserName( const std::string& buffer ) const;
		std::string getMessage( const std::string& buffer ) const;
		std::string getChannelName( const std::string& buffer ) const;

		std::string&	rtrim( std::string& s ) const;
};
