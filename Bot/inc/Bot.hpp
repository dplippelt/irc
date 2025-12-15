/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 10:37:54 by dlippelt          #+#    #+#             */
/*   Updated: 2025/12/15 16:55:25 by dlippelt         ###   ########.fr       */
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
#include <signal.h>

#include "Game.hpp"
#include "MPGame.hpp"
#include "BotCommands.hpp"

extern volatile sig_atomic_t g_quit;

class Bot
{
	public:
		~Bot();
		Bot() = delete;
		Bot( const std::string& server_port, std::string_view pw );
		Bot( const Bot& ) = delete;
		Bot& operator=( const Bot& ) = delete;

		static void sigHandler(int signum);
		static void setupSigHandler();

		enum BotIRCCommandType
		{
			CMD_JOIN,
			CMD_PART,
			CMD_QUIT,
			CMD_KICK,
			CMD_UNKNOWN,
		};

		static inline const std::map<std::string, BotIRCCommandType> k_commands
		{
			{"JOIN", CMD_JOIN},
			{"PART", CMD_PART},
			{"QUIT", CMD_QUIT},
			{"KICK", CMD_KICK},
		};

		void	doPoll();

		int																getSocket() const;
		const std::map<std::string, Game*>&								getGames() const;
		void															removeGame( const std::string& username );
		void															addGame( const std::string& username, Game* game );

		const std::map<std::pair<std::string, std::string>, MPGame*>&		getMPGames() const;
		std::map<std::pair<std::string, std::string>, MPGame *>::iterator	getMPGame( const std::string& player_one, const std::string& player_two );
		void																addMPGame( const std::pair<std::string, std::string>& usernames, MPGame* mp_game );
		void																removeMPGame( const std::string& player_one, const std::string& player_two );
		const std::vector<std::pair<std::string, std::string>>&				getChallenges() const;
		void																addChallenge( const std::string& player_one, const std::string& player_two );
		void																removeChallenge( const std::string& player_one, const std::string& player_two );
		bool																memberInChannel( const std::string& username ) const;

	private:
		std::string			m_pw {};
		struct addrinfo*	m_server_addr {};
		int					m_bot_socket_fd {};
		struct pollfd		m_pollfd {};

		std::vector<std::string>								m_welcomed {};
		std::map<std::string, Game*>							m_games {};
		std::map<std::pair<std::string, std::string>, MPGame*>	m_mp_games {};
		std::vector<std::pair<std::string, std::string>>		m_challenges {};
		std::vector<std::string>								m_channel_members {};

		void	validatePort( const std::string& port ) const;
		void	authenticateAndJoin() const;
		void	receiveMessage();
		void	processBuffer( const std::string& buffer );

		std::string	getUserName( const std::string& buffer ) const;
		std::string getMessage( const std::string& buffer ) const;
		std::string getChannelName( const std::string& buffer ) const;
		std::string getIRCCommand( const std::string& buffer ) const;

		bool	needWelcome( const std::string& irc_cmd, const std::string& username );
		void	trackChannelMembers( const std::string& username, const std::string& irc_cmd );
		void	addChannelMember( const std::string& username );
		void	removeChannelMember( const std::string& username );

		std::string&		rtrim( std::string& s ) const;
		BotIRCCommandType	getIRCCmdType( const std::string& irc_cmd ) const;
};
