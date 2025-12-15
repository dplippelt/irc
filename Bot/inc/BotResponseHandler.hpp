/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BotResponseHandler.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 17:27:31 by dlippelt          #+#    #+#             */
/*   Updated: 2025/12/15 18:04:58 by dlippelt         ###   ########.fr       */
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
		~BotResponseHandler();
		BotResponseHandler() = delete;
		BotResponseHandler( Bot& bot );
		BotResponseHandler( const BotResponseHandler& ) = delete;
		BotResponseHandler& operator=( const BotResponseHandler& ) = delete;

		Bot&	m_bot;
		int		m_bot_socket_fd;

		enum GridType
		{
			REFERENCE,
			TRACKING,
			SOLUTION,
			NONE,
		};

		void	sendPlayerGrid( const std::string& username, const Grid& grid, GridType type = NONE, const std::string& opponent = "");
		void	sendStart( const std::string& username, const std::string& channel, const Grid& grid );
		void	sendFire( const std::string& username, const std::string& channel, const Grid& grid );
		void	sendBoard( const std::string& username, const std::string& channel, const Grid& grid );
		void	sendSolution( const std::string& username, const std::string& channel, const Grid& grid );
		void	sendNewGame( const std::string& username, const std::string& channel, const Grid& grid );
		void	sendUnknownCmdFeedback( const std::string& username, const std::string& channel, const std::string& cmd );
		void	sendGameAlreadyRunningFeedback( const std::string& username, const std::string& channel );
		void	sendNoGameFeedback( const std::string& username, const std::string& channel );
		void	sendNoTargetFeedback( const std::string& username, const std::string& channel );
		void	sendInvalidTargetFeedback( const std::string& username, const std::string& channel, const std::string& target );
		void	sendMissFeedback( const std::string& username, const std::string& target, const std::string& opponent = "" );
		void	sendHitFeedback( const std::string& username, const std::string& target, const std::string& opponent = "" );
		void	sendSunkFeedback( const std::string& username, const std::string& sunkName, const std::string& opponent = "" );
		void	sendWonFeedback( const std::string& username, const std::string& opponent = "" );
		void	sendHelp( const std::string& username, const std::pair<std::string, std::string>& cmd );
		void	sendWelcome( const std::string& username, const std::string& channel );

		// MP specific responses
		void	sendChallenge( const std::string& challenger, const std::string& challenged, const std::string& channel );
		void	sendAccept( const std::string& challenger, const std::string& challenged, const std::string& channel );
		void	sendNoChallengedFeedback( const std::string& username, const std::string& channel );
		void	sendNoChallengerFeedback( const std::string& username, const std::string& channel );
		void	sendAlreadyChallengedFeedback( const std::string& username, const std::string& channel, const std::string& challenged );
		void	sendNoChallengeToAcceptFeedback( const std::string& username, const std::string& channel, const std::string& challenger );
		void	sendMPGameAlreadyRunningFeedback( const std::string& challenger, const std::string& challenged, const std::string& channel );
		void	sendCannotChallengeSelfFeedback( const std::string& username, const std::string& channel );
		void	sendCannotAcceptSelfFeedback( const std::string& username, const std::string& channel );
		void	sendNoMPGameFeedback( const std::string& username, const std::string& opponent, const std::string& channel );
		void	sendNotYourTurnFeedback( const std::string& username, const std::string& opponent, const std::string& channel );
		void	sendTurnInfo( const std::string& player_one, const std::string& player_two, const MPGame* mp_game );
		void	sendCannotSurrenderToSelfFeedback( const std::string& username, const std::string& channel );
		void	sendSurrender( const std::string& username, const std::string& opponent, const std::string& channel );
		void	sendCannotShowFleetOrShotsAgainstSelf( const std::string& username, const std::string& channel );
		void	sendCannotShootSelf( const std::string& username, const std::string& channel );
		void	sendShot( const std::string& username, const std::string& channel, const std::string& opponent, const Grid& playerShots, const Grid& opponentFleet );
		void	sendFleet( const std::string& username, const std::string& channel, const std::string& opponent, const Grid& grid );
		void	sendShots( const std::string& username, const std::string& channel, const std::string& opponent, const Grid& grid );


		// Helpers
		void		sendResponse( const std::string& username, const std::string& channel, const std::string& msg );
		std::string	capitalize( const std::string& target );
};
