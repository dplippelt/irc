/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BotResponseHandler.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 17:32:25 by dlippelt          #+#    #+#             */
/*   Updated: 2025/12/15 18:10:32 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BotResponseHandler.hpp"

/* ==================== Constructors & Destructors ==================== */

BotResponseHandler::~BotResponseHandler() = default;

BotResponseHandler::BotResponseHandler( Bot& bot )
	: m_bot {bot}
	, m_bot_socket_fd { bot.getSocket() }
{
}

/* ===================== Bot Response ===================== */

void	BotResponseHandler::sendPlayerGrid( const std::string& username, const Grid& grid, GridType type, const std::string& opponent )
{
	const std::string	gridMsg { grid.getGridMsg() };
	std::istringstream	iss { gridMsg };
	std::string			line {};

	sendResponse(username, "", "");
	if ( type == GridType::TRACKING )
		sendResponse(username, "", "Your " COLOR RED "shots" RESET " vs " COLOR RED + opponent + RESET "." );
	else if ( type == GridType::REFERENCE )
		sendResponse(username, "", "Your " COLOR GREEN "fleet" RESET " vs " COLOR RED + opponent + RESET "." );
	else if ( type == GridType::SOLUTION )
		sendResponse(username, "", "Battleships game solution for " COLOR RED + username + "'s " RESET "game");
	else
		sendResponse(username, "", "Your single player Battleships grid");
	sendResponse(username, "", "");

	while ( std::getline(iss, line) )
		sendResponse(username, "", line);
	sendResponse(username, "", "");
}

void	BotResponseHandler::sendStart( const std::string& username, const std::string& channel, const Grid& grid )
{
	if ( !channel.empty() )
		BotResponseHandler::sendResponse(username, channel, "Your single player game has begun, " COLOR RED + username + RESET ". Your grid was sent to your DMs.");
	BotResponseHandler::sendPlayerGrid(username, grid);
}

void	BotResponseHandler::sendFire( const std::string& username, const std::string& channel, const Grid& grid )
{
	if ( !channel.empty() )
		BotResponseHandler::sendResponse(username, channel, "The result of your shot was sent to your DMs, " COLOR RED + username + RESET ".");
	BotResponseHandler::sendPlayerGrid(username, grid);
}

void	BotResponseHandler::sendBoard( const std::string& username, const std::string& channel, const Grid& grid )
{
	if ( !channel.empty() )
		BotResponseHandler::sendResponse(username, channel, "Your game board was sent to your DMs, " COLOR RED + username + RESET ".");
	BotResponseHandler::sendPlayerGrid(username, grid);
}

void	BotResponseHandler::sendSolution( const std::string& username, const std::string& channel, const Grid& grid )
{
	if ( !channel.empty() )
		BotResponseHandler::sendResponse(username, channel, "The solution to your single player game was sent to your DMs, " COLOR RED + username + RESET ".");
	BotResponseHandler::sendPlayerGrid(username, grid, GridType::SOLUTION);
}

void	BotResponseHandler::sendNewGame( const std::string& username, const std::string& channel, const Grid& grid )
{
	if ( !channel.empty() )
		BotResponseHandler::sendResponse(username, channel, "Your single player game was reset, " COLOR RED + username + RESET ". A new grid was sent to your DMs.");
	BotResponseHandler::sendPlayerGrid(username, grid);
}

void BotResponseHandler::sendUnknownCmdFeedback( const std::string& username, const std::string& channel, const std::string& cmd )
{
	sendResponse(username, channel, "");
	sendResponse(username, channel, "Unknow command: " COLOR RED + cmd + RESET);
	sendResponse(username, channel, "Please type " COLOR YELLOW "!help" RESET " for a list of commands.");
	sendResponse(username, channel, "");
}

void BotResponseHandler::sendGameAlreadyRunningFeedback( const std::string& username, const std::string& channel )
{
	sendResponse(username, channel, "");
	sendResponse(username, channel, "You already have a Battleships game running!");
	sendResponse(username, channel, "Type " COLOR YELLOW "!board" RESET " to see your current game board or " COLOR YELLOW "!newgame" RESET " to start a fresh game.");
	sendResponse(username, channel, "");
}

void BotResponseHandler::sendNoGameFeedback( const std::string& username, const std::string& channel )
{
	sendResponse(username, channel, "");
	sendResponse(username, channel, "You need to start a game before you can use this command. You can start a new game by typing " COLOR YELLOW "!start" RESET ".");
	sendResponse(username, channel, "");
}

void BotResponseHandler::sendNoTargetFeedback( const std::string& username, const std::string& channel )
{
	sendResponse(username, channel, "");
	sendResponse(username, channel, "Please specify a target (e.g. " COLOR YELLOW "!fire B3" RESET ")");
	sendResponse(username, channel, "");
}

void BotResponseHandler::sendInvalidTargetFeedback( const std::string& username, const std::string& channel, const std::string& target )
{
	sendResponse(username, channel, "");
	sendResponse(username, channel, "This is not a valid target: " COLOR RED + capitalize(target) + RESET);
	sendResponse(username, channel, "");
}

void BotResponseHandler::sendHelp( const std::string& username, const std::pair<std::string, std::string>& cmd )
{
	sendResponse(username, "", cmd.first);
	sendResponse(username, "", cmd.second);
	sendResponse(username, "", "");
}

void BotResponseHandler::sendMissFeedback( const std::string& username, const std::string& target, const std::string& opponent )
{
	if (opponent.length())
	{
		sendResponse(opponent, "", COLOR RED + username + "'s" RESET " shot at " COLOR YELLOW + capitalize(target) + COLOR CYAN " missed" RESET "!");
		sendResponse(opponent, "", "");
	}

	sendResponse(username, "", "Your shot at " COLOR YELLOW + capitalize(target) + COLOR CYAN " missed" RESET "!");
	sendResponse(username, "", "");
}

void BotResponseHandler::sendHitFeedback( const std::string& username, const std::string& target, const std::string& opponent )
{
	if (opponent.length())
	{
		sendResponse(opponent, "", COLOR RED + username + "'s" RESET " shot at " COLOR YELLOW + capitalize(target) + RESET " was a " COLOR RED "hit" RESET "!");
		sendResponse(username, "", "You " COLOR RED "hit" RESET " one of " COLOR RED + opponent + "'s" RESET " ships at " COLOR YELLOW + capitalize(target) + RESET "!");
		sendResponse(opponent, "", "");
		sendResponse(username, "", "");
	}
	else
	{
		sendResponse(username, "", "You " COLOR RED "hit" RESET " an enemy ship at " COLOR YELLOW + capitalize(target) + RESET "!");
		sendResponse(username, "", "");
	}
}

void BotResponseHandler::sendSunkFeedback( const std::string& username, const std::string& sunkName, const std::string& opponent )
{
	if (opponent.length())
	{
		sendResponse(opponent, "", BOLD COLOR RED + username + " sunk your " + sunkName + "!" RESET);
		sendResponse(username, "", BOLD COLOR PURPLE "You sunk " + opponent + "'s " + sunkName + "! Congrats, keep going!" RESET);
		sendResponse(opponent, "", "");
		sendResponse(username, "", "");
	}
	else
	{
		sendResponse(username, "", BOLD COLOR PURPLE "You sunk the enemy's " + sunkName + "! Congrats, keep going!" RESET);
		sendResponse(username, "", "");
	}
}

void BotResponseHandler::sendWonFeedback( const std::string& username, const std::string& opponent )
{
	if (opponent.length())
	{
		sendResponse(username, "", BOLD COLOR GREEN "You sunk all of " + opponent + "'s ships and won the game!" RESET);
		sendResponse(username, "", "");
		sendResponse(username, "", "To play against " COLOR RED + opponent + RESET " again type " COLOR YELLOW "!challenge " + opponent + RESET " to challenge them to another round.");
		sendResponse(username, "", "");

		sendResponse(opponent, "", BOLD COLOR RED "Oof, " + username + " sunk all of your ships. You have been defeated." RESET);
		sendResponse(opponent, "", "");
		sendResponse(opponent, "", "To challenge " COLOR RED + username + RESET " to a rematch type " COLOR YELLOW "!challenge " + username + RESET ". You'll get 'em this time!");
		sendResponse(opponent, "", "");
	}
	else
	{
		sendResponse(username, "", BOLD COLOR GREEN "Well done, you sunk all of the enemy's ships!" RESET);
		sendResponse(username, "", "");
		sendResponse(username, "", "To play again just type " COLOR YELLOW "!start" RESET " in the Battleships channel or as a private message to BattleShipsBot.");
		sendResponse(username, "", "");
	}
}

void BotResponseHandler::sendWelcome( const std::string& username, const std::string& channel )
{
	sendResponse(username, channel, "");
	sendResponse(username, channel, BOLD COLOR LIGHT_CYAN "Hello, I am the BattleshipsBot, " + username + "!" RESET);
	sendResponse(username, channel, BOLD COLOR LIGHT_CYAN "To start a single player game type " COLOR YELLOW "!start" COLOR LIGHT_CYAN " and " COLOR YELLOW "!fire <target>" COLOR LIGHT_CYAN " (e.g., !fire B3) to shoot at a target on the grid." RESET);
	sendResponse(username, channel, BOLD COLOR LIGHT_CYAN "To challenge another user to a multiplayer match type " COLOR YELLOW "!challenge <username>" COLOR LIGHT_CYAN " (e.g., !challenge bob). To accept another users challenge type " COLOR YELLOW "!accept <username>" COLOR LIGHT_CYAN ". In multiplayer use the command " COLOR YELLOW "!shoot <username> <target>" COLOR LIGHT_CYAN " to take a shot at another player's fleet (e.g., !shoot bob A3)." RESET);
	sendResponse(username, channel, BOLD COLOR LIGHT_CYAN "For a full list of commands type " COLOR YELLOW "!help" COLOR LIGHT_CYAN "." RESET);
	sendResponse(username, channel, "");
}





/* ===================== Bot Response (Multiplayer specific) ===================== */

void	BotResponseHandler::sendChallenge( const std::string& challenger, const std::string& challenged, const std::string& channel )
{
	if ( !channel.empty() )
	{
		sendResponse("", channel, "");
		sendResponse("", channel, COLOR RED + challenger + RESET " challenges " COLOR RED + challenged + RESET " to a game of Battleships!");
		sendResponse("", channel, "To accept the challenge and start playing, " COLOR RED + challenged + RESET ", please type " COLOR YELLOW "!accept " + challenger + RESET ".");
		sendResponse("", channel, "");
	}

	if ( !m_bot.memberInChannel(challenger) || channel.empty())
	{
		sendResponse(challenger, "", "");
		sendResponse(challenger, "", "You have challenged " COLOR RED + challenged + RESET " to a game of Battleships!");
		sendResponse(challenger, "", "They have been notified and will need to accept your challenge before the game can begin.");
		sendResponse(challenger, "", "");
	}

	if ( !m_bot.memberInChannel(challenged) || channel.empty() )
	{
		sendResponse(challenged, "", "");
		sendResponse(challenged, "", "You have been challenged to a game of Battleships by " COLOR RED + challenger + RESET "!");
		sendResponse(challenged, "", "To accept the challenge and start playing type " COLOR YELLOW "!accept " + challenger + RESET ".");
		sendResponse(challenged, "", "");
	}
}

void	BotResponseHandler::sendNoChallengedFeedback( const std::string& username, const std::string& channel )
{
	sendResponse(username, channel, "");
	sendResponse(username, channel, "Please specify the name of the user you would like to challenge (e.g. " COLOR YELLOW "!challenge bob" RESET ").");
	sendResponse(username, channel, "");
}

void	BotResponseHandler::sendAlreadyChallengedFeedback( const std::string& username, const std::string& channel, const std::string& challenged )
{
	sendResponse(username, channel, "");
	sendResponse(username, channel, "You have already challenged " COLOR RED + challenged + RESET " or they have already challenged you to a game of Battleships. Please either accept their challenge or wait for them to accept your challenge.");
	sendResponse(username, channel, "");
}

void	BotResponseHandler::sendAccept( const std::string& challenger, const std::string& challenged, const std::string& channel )
{
	if ( !channel.empty() )
	{
		sendResponse("", channel, "");
		sendResponse("", channel, COLOR RED + challenged + RESET " has accepted " COLOR RED + challenger + RESET "'s challenge to a game of Battleships! The game will now begin.");
		sendResponse("", channel, "Your grids have been sent to your DMs.");
		sendResponse("", channel, "");
	}

	if ( !m_bot.memberInChannel(challenger) || channel.empty() )
	{
		sendResponse(challenger, "", "");
		sendResponse(challenger, "", COLOR RED + challenged + RESET " has accepted your challenge! The game will now begin.");
		sendResponse(challenger, "", "");
	}

	if ( !m_bot.memberInChannel(challenged) || channel.empty() )
	{
		sendResponse(challenged, "", "");
		sendResponse(challenged, "", "You have accepted " COLOR RED + challenger + RESET "'s challenge! The game will now begin.");
		sendResponse(challenged, "", "");
	}
}

void	BotResponseHandler::sendCannotChallengeSelfFeedback( const std::string& username, const std::string& channel )
{
	sendResponse(username, channel, "");
	sendResponse(username, channel, "You cannot challenge yourself to a game of Battleships...");
	sendResponse(username, channel, "");
}

void	BotResponseHandler::sendCannotAcceptSelfFeedback( const std::string& username, const std::string& channel )
{
	sendResponse(username, channel, "");
	sendResponse(username, channel, "You cannot accept a challenge to a game of Battleships from yourself...");
	sendResponse(username, channel, "");
}

void	BotResponseHandler::sendNoChallengerFeedback( const std::string& username, const std::string& channel )
{
	sendResponse(username, channel, "");
	sendResponse(username, channel, "Please specify the name of the user whom's challenge you would like to accept (e.g. " COLOR YELLOW "!accept alice" RESET ").");
	sendResponse(username, channel, "");
}

void	BotResponseHandler::sendNoChallengeToAcceptFeedback( const std::string& username, const std::string& channel,const std::string& challenger )
{
	sendResponse(username, channel, "");
	sendResponse(username, channel, "The user " COLOR RED + challenger + RESET " has not challenged you to a game of Battleships yet.");
	sendResponse(username, channel, "Please wait for them to send you a challenge before accepting.");
	sendResponse(username, channel, "Alternatively, you can challenge them yourself by typing " COLOR YELLOW "!challenge " + challenger + RESET ".");
	sendResponse(username, channel, "");
}

void	BotResponseHandler::sendMPGameAlreadyRunningFeedback( const std::string& username, const std::string& opponent, const std::string& channel )
{
	if ( !channel.empty() )
	{
		sendResponse("", channel, "");
		sendResponse("", channel, "There is already a Battleships game in progress between " COLOR RED + username + RESET " and " COLOR RED + opponent + RESET ".");
		sendResponse("", channel, COLOR RED + username + RESET ": to see your current game board type " COLOR YELLOW "!shots " + opponent + RESET ".");
		sendResponse("", channel, "");

		return;
	}

	sendResponse(username, "", "");
	sendResponse(username, "", "There is already a Battleships game in progress between you and " COLOR RED + opponent + RESET ".");
	sendResponse(username, "", "Type " COLOR YELLOW "!shots " + opponent + RESET " to see your current game board.");
	sendResponse(username, "", "");
}

void	BotResponseHandler::sendNoMPGameFeedback( const std::string& username, const std::string& opponent, const std::string& channel )
{
	sendResponse(username, channel, "");
	sendResponse(username, channel, "There is no Battleships game running between you and '" COLOR RED + opponent + RESET "' at the moment.");
	sendResponse(username, channel, "You can challenge them to a game by typing " COLOR YELLOW "!challenge " + opponent + RESET ".");
	sendResponse(username, channel, "");
}

void	BotResponseHandler::sendNotYourTurnFeedback( const std::string& username, const std::string& opponent, const std::string& channel )
{
	sendResponse(username, channel, "");
	sendResponse(username, channel, "It is not your turn! Please wait for " COLOR RED + opponent + RESET " to take their shot.");
	sendResponse(username, channel, "");
}

void	BotResponseHandler::sendTurnInfo( const std::string& player_one, const std::string& player_two, const MPGame* mp_game )
{
	const std::string&	current_player { mp_game->getCurrentPlayer() };

	if ( player_one == current_player )
	{
		sendResponse(player_one, "", "It is your turn, " COLOR RED + player_one + RESET "!");
		sendResponse(player_one, "", "");

		sendResponse(player_two, "", "It is " COLOR RED + player_one + "'s" RESET " turn.");
		sendResponse(player_two, "", "");
	}
	else
	{
		sendResponse(player_two, "", "It is your turn, " COLOR RED + player_two + RESET "!");
		sendResponse(player_two, "", "");

		sendResponse(player_one, "", "It is " COLOR RED + player_two + "'s" RESET " turn.");
		sendResponse(player_one, "", "");
	}
}

void	BotResponseHandler::sendCannotSurrenderToSelfFeedback( const std::string& username, const std::string& channel )
{
	sendResponse(username, channel, "");
	sendResponse(username, channel, "You cannot surrender to yourself...");
	sendResponse(username, channel, "");
}

void	BotResponseHandler::sendSurrender( const std::string& username, const std::string& opponent, const std::string& channel )
{
	if ( !channel.empty() )
	{
		sendResponse("", channel, "");
		sendResponse("", channel, COLOR RED + username + RESET " publicly surrenders to " COLOR RED + opponent + RESET ".");
		sendResponse("", channel, COLOR GREEN + opponent + " has won the match!" RESET);
		sendResponse("", channel, "");
	}

	if ( !m_bot.memberInChannel(username) || channel.empty() )
	{
		sendResponse(username, "", "");
		sendResponse(username, "", COLOR RED "You have surrendered your fleet to " + opponent + ". You lost the battle." RESET);
		sendResponse(username, "", "");
	}

	if ( !m_bot.memberInChannel(opponent) || channel.empty() )
	{
		sendResponse(opponent, "", "");
		sendResponse(opponent, "", COLOR GREEN + username + " has surrendered! You have won the game!" RESET);
		sendResponse(opponent, "", "");
	}
}

void	BotResponseHandler::sendCannotShootSelf( const std::string& username, const std::string& channel )
{
	sendResponse(username, channel, "");
	sendResponse(username, channel, "Hey, watch where you point that thing, the enemy is over there!");
	sendResponse(username, channel, "");
}

void	BotResponseHandler::sendCannotShowFleetOrShotsAgainstSelf( const std::string& username, const std::string& channel )
{
	sendResponse(username, channel, "");
	sendResponse(username, channel, "Please specify the name of the user your fleet is fighting, not yourself.");
	sendResponse(username, channel, "");
}

void	BotResponseHandler::sendShot( const std::string& username, const std::string& channel, const std::string& opponent, const Grid& playerShots, const Grid& opponentFleet )
{
	if ( !channel.empty() )
		BotResponseHandler::sendResponse("", channel, "You fired a shot at " + opponent + "'s fleet, " COLOR RED + username + RESET ". Check your DMs for the shot result.");
	BotResponseHandler::sendPlayerGrid(username, playerShots, BotResponseHandler::GridType::TRACKING, opponent);
	BotResponseHandler::sendPlayerGrid(opponent, opponentFleet, BotResponseHandler::GridType::REFERENCE, username);
}

void	BotResponseHandler::sendFleet( const std::string& username, const std::string& channel, const std::string& opponent, const Grid& grid )
{
	if ( !channel.empty() )
		BotResponseHandler::sendResponse(username, channel, "A grid with your fleet was sent to your DMs, " COLOR RED + username + RESET ".");
	BotResponseHandler::sendPlayerGrid(username, grid, BotResponseHandler::GridType::REFERENCE, opponent);
}

void	BotResponseHandler::sendShots( const std::string& username, const std::string& channel, const std::string& opponent, const Grid& grid )
{
	if ( !channel.empty() )
		BotResponseHandler::sendResponse(username, channel, "Your shots grid was sent to your DMs, " COLOR RED + username + RESET ".");
	BotResponseHandler::sendPlayerGrid(username, grid, BotResponseHandler::GridType::TRACKING, opponent);
}





/* ===================== Helpers ===================== */

void	BotResponseHandler::sendResponse( const std::string& username, const std::string& channel, const std::string& msg )
{
	std::string	prefix {};
	std::string	response {};

	if (channel.length())
		prefix = "PRIVMSG " +  channel + " :";
	else
		prefix = "PRIVMSG " +  username + " :";

	response = prefix + msg + "\r\n";

	send(m_bot_socket_fd, response.data(), response.length(), 0);
}

std::string	BotResponseHandler::capitalize( const std::string& target )
{
	std::string capTarget {};

	capTarget = target;
	capTarget[0] = std::toupper(capTarget[0]);

	return capTarget;
}
