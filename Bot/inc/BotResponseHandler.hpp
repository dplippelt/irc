/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BotResponseHandler.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 17:27:31 by dlippelt          #+#    #+#             */
/*   Updated: 2025/12/04 17:19:01 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include "Grid.hpp"
#include "Bot.hpp"
#include "macros.hpp"

class Bot;

class BotResponseHandler
{
	public:
		BotResponseHandler() = delete;

		static void	sendPlayerGrid( int bot_socket_fd, const std::string& username, const std::string& channel, const Grid& grid );
		static void	sendSolution( int bot_socket_fd, const std::string& username, const std::string& channel, const Grid& grid );
		static void sendUnknownCmdFeedback( int bot_socket_fd, const std::string& username, const std::string& channel, const std::string& cmd );
		static void sendGameAlreadyRunningFeedback( int bot_socket_fd, const std::string& username, const std::string& channel );
		static void sendNoGameFeedback( int bot_socket_fd, const std::string& username, const std::string& channel );
		static void sendNoTargetFeedback( int bot_socket_fd, const std::string& username, const std::string& channel );
		static void sendInvalidTargetFeedback( int bot_socket_fd, const std::string& username, const std::string& channel, const std::string& target );
		static void sendMissFeedback( int bot_socket_fd, const std::string& username, const std::string& channel, const std::string& target );
		static void sendHitFeedback( int bot_socket_fd, const std::string& username, const std::string& channel, const std::string& target );
		static void sendSunkFeedback( int bot_socket_fd, const std::string& username, const std::string& channel, const std::string& sunkName );
		static void sendWonFeedback( int bot_socket_fd, const std::string& username, const std::string& channel );
		static void sendHelp( int bot_socket_fd, const std::string& username, const std::string& channel, const std::pair<std::string, std::string>& cmd );
		static void sendWelcome( int bot_socket_fd, const std::string& username, const std::string& channel );

		// MP response handler
		static void sendChallenge( const Bot& bot, const std::string& challenger, const std::string& challenged, const std::string& channel );
		static void sendAccept( const Bot& bot, const std::string& challenger, const std::string& challenged, const std::string& channel );
		static void sendNoChallengedFeedback( const Bot& bot, const std::string& username, const std::string& channel );
		static void sendNoChallengerFeedback( const Bot& bot, const std::string& username, const std::string& channel );
		static void sendAlreadyChallengedFeedback( const Bot& bot, const std::string& username, const std::string& channel,const std::string& challenged );
		static void sendNoChallengeToAcceptFeedback( const Bot& bot, const std::string& username, const std::string& channel, const std::string& challenger );
		static void sendMPGameAlreadyRunningFeedback( const Bot& bot, const std::string& challenger, const std::string& challenged, const std::string& channel );



		static void	sendResponse( int bot_socket_fd, const std::string& username, const std::string& channel, const std::string& msg );
		static std::string	capitalize( const std::string& target );
};
