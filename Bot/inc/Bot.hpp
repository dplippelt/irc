/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 10:37:54 by dlippelt          #+#    #+#             */
/*   Updated: 2025/12/01 12:46:19 by dlippelt         ###   ########.fr       */
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
#include "MPGame.hpp"
#include "BotCommands.hpp"

class Bot
{
	public:
		~Bot();
		Bot() = delete;
		Bot( const std::string& server_port, std::string_view pw );
		Bot( const Bot& ) = delete;
		Bot& operator=( const Bot& ) = delete;

		void	doPoll();

		int																getSocket() const;
		const std::map<std::string, Game*>								getGames() const;
		void															removeGame( std::string username );
		void															addGame( std::string username, Game* game );

		const std::map<std::pair<std::string, std::string>, MPGame*>	getMPGames() const;
		void															addMPGame( std::pair<std::string, std::string> usernames, MPGame* mp_game );
		void															removeMPGame( std::pair<std::string, std::string> usernames );

	private:
		std::string			m_pw {};
		struct addrinfo*	m_server_addr {};
		int					m_bot_socket_fd {};
		struct pollfd		m_pollfd {};

		std::vector<std::string>								m_welcomed {};
		std::map<std::string, Game*>							m_games {};
		std::map<std::pair<std::string, std::string>, MPGame*>	m_mp_games {};

		void	validatePort( const std::string& port ) const;
		void	authenticateAndJoin() const;
		void	receiveMessage();
		void	processBuffer( const std::string& buffer );

		std::string	getUserName( const std::string& buffer ) const;
		std::string getMessage( const std::string& buffer ) const;
		std::string getChannelName( const std::string& buffer ) const;
		std::string getIRCCommand( const std::string& buffer ) const;

		bool	needWelcome( const std::string& irc_cmd, const std::string& username );

		std::string&	rtrim( std::string& s ) const;
};
