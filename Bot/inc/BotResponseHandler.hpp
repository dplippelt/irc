/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BotResponseHandler.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 17:27:31 by dlippelt          #+#    #+#             */
/*   Updated: 2025/12/06 11:21:27 by dlippelt         ###   ########.fr       */
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

		enum GridType
		{
			REFERENCE,
			TRACKING,
			SOLUTION,
			NONE,
		};

		static void	sendPlayerGrid( int bot_socket_fd, const std::string& username, const Grid& grid, GridType type = NONE, const std::string& opponent = "");
		static void	sendStart( int bot_socket_fd, const std::string& username, const std::string& channel, const Grid& grid );
		static void	sendFire( int bot_socket_fd, const std::string& username, const std::string& channel, const Grid& grid );
		static void	sendBoard( int bot_socket_fd, const std::string& username, const std::string& channel, const Grid& grid );
		static void	sendSolution( int bot_socket_fd, const std::string& username, const std::string& channel, const Grid& grid );
		static void	sendNewGame( int bot_socket_fd, const std::string& username, const std::string& channel, const Grid& grid );
		static void sendUnknownCmdFeedback( int bot_socket_fd, const std::string& username, const std::string& channel, const std::string& cmd );
		static void sendGameAlreadyRunningFeedback( int bot_socket_fd, const std::string& username, const std::string& channel );
		static void sendNoGameFeedback( int bot_socket_fd, const std::string& username, const std::string& channel );
		static void sendNoTargetFeedback( int bot_socket_fd, const std::string& username, const std::string& channel );
		static void sendInvalidTargetFeedback( int bot_socket_fd, const std::string& username, const std::string& channel, const std::string& target );
		static void sendMissFeedback( int bot_socket_fd, const std::string& username, const std::string& target, const std::string& opponent = "" );
		static void sendHitFeedback( int bot_socket_fd, const std::string& username, const std::string& target, const std::string& opponent = "" );
		static void sendSunkFeedback( int bot_socket_fd, const std::string& username, const std::string& sunkName, const std::string& opponent = "" );
		static void sendWonFeedback( int bot_socket_fd, const std::string& username, const std::string& opponent = "" );
		static void sendHelp( int bot_socket_fd, const std::string& username, const std::pair<std::string, std::string>& cmd );
		static void sendWelcome( int bot_socket_fd, const std::string& username, const std::string& channel );

		// MP specific responses
		static void sendChallenge( const Bot& bot, const std::string& challenger, const std::string& challenged, const std::string& channel );
		static void sendAccept( const Bot& bot, const std::string& challenger, const std::string& challenged, const std::string& channel );
		static void sendNoChallengedFeedback( const Bot& bot, const std::string& username, const std::string& channel );
		static void sendNoChallengerFeedback( const Bot& bot, const std::string& username, const std::string& channel );
		static void sendAlreadyChallengedFeedback( const Bot& bot, const std::string& username, const std::string& channel, const std::string& challenged );
		static void sendNoChallengeToAcceptFeedback( const Bot& bot, const std::string& username, const std::string& channel, const std::string& challenger );
		static void sendMPGameAlreadyRunningFeedback( const Bot& bot, const std::string& challenger, const std::string& challenged, const std::string& channel );
		static void sendCannotChallengeSelfFeedback( const Bot& bot, const std::string& username, const std::string& channel );
		static void sendCannotAcceptSelfFeedback( const Bot& bot, const std::string& username, const std::string& channel );
		static void	sendNoMPGameFeedback( const Bot& bot, const std::string& username, const std::string& opponent, const std::string& channel );
		static void	sendNotYourTurnFeedback( const Bot& bot, const std::string& username, const std::string& opponent, const std::string& channel );
		static void	sendTurnInfo( const Bot& bot, const std::string& player_one, const std::string& player_two, const MPGame* mp_game );
		static void sendCannotSurrenderToSelfFeedback( const Bot& bot, const std::string& username, const std::string& channel );
		static void sendSurrender( const Bot& bot, const std::string& username, const std::string& opponent, const std::string& channel );
		static void sendCannotShowFleetOrShotsAgainstSelf( const Bot& bot, const std::string& username, const std::string& channel );
		static void sendCannotShootSelf( const Bot& bot, const std::string& username, const std::string& channel );
		static void	sendShot( const Bot& bot, const std::string& username, const std::string& channel, const std::string& opponent, const Grid& playerShots, const Grid& opponentFleet );
		static void	sendFleet( const Bot& bot, const std::string& username, const std::string& channel, const std::string& opponent, const Grid& grid );
		static void	sendShots( const Bot& bot, const std::string& username, const std::string& channel, const std::string& opponent, const Grid& grid );


		// Helpers
		static void	sendResponse( int bot_socket_fd, const std::string& username, const std::string& channel, const std::string& msg );
		static std::string	capitalize( const std::string& target );
};
