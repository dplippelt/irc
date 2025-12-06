/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BotResponseHandler.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 17:32:25 by dlippelt          #+#    #+#             */
/*   Updated: 2025/12/06 12:37:06 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BotResponseHandler.hpp"

/* ===================== Bot Response ===================== */

void	BotResponseHandler::sendPlayerGrid( int bot_socket_fd, const std::string& username, const Grid& grid, GridType type, const std::string& opponent )
{
	const std::string	gridMsg { grid.getGridMsg() };
	std::istringstream	iss { gridMsg };
	std::string			line {};

	sendResponse(bot_socket_fd, username, "", "");
	if ( type == GridType::TRACKING )
		sendResponse(bot_socket_fd, username, "", "Your " COLOR RED "shots" RESET " vs " COLOR RED + opponent + RESET "." );
	else if ( type == GridType::REFERENCE )
		sendResponse(bot_socket_fd, username, "", "Your " COLOR GREEN "fleet" RESET " vs " COLOR RED + opponent + RESET "." );
	else if ( type == GridType::SOLUTION )
		sendResponse(bot_socket_fd, username, "", "Battleships game solution for " COLOR RED + username + "'s " RESET "game");
	else
		sendResponse(bot_socket_fd, username, "", "Your single player Battleships grid");
	sendResponse(bot_socket_fd, username, "", "");

	while ( std::getline(iss, line) )
		sendResponse(bot_socket_fd, username, "", line);
	sendResponse(bot_socket_fd, username, "", "");
}

void	BotResponseHandler::sendStart( int bot_socket_fd, const std::string& username, const std::string& channel, const Grid& grid )
{
	if ( !channel.empty() )
		BotResponseHandler::sendResponse(bot_socket_fd, username, channel, "Your single player game has begun, " COLOR RED + username + RESET ". Your grid was sent to your DMs.");
	BotResponseHandler::sendPlayerGrid(bot_socket_fd, username, grid);
}

void	BotResponseHandler::sendFire( int bot_socket_fd, const std::string& username, const std::string& channel, const Grid& grid )
{
	if ( !channel.empty() )
		BotResponseHandler::sendResponse(bot_socket_fd, username, channel, "The result of your shot was sent to your DMs, " COLOR RED + username + RESET ".");
	BotResponseHandler::sendPlayerGrid(bot_socket_fd, username, grid);
}

void	BotResponseHandler::sendBoard( int bot_socket_fd, const std::string& username, const std::string& channel, const Grid& grid )
{
	if ( !channel.empty() )
		BotResponseHandler::sendResponse(bot_socket_fd, username, channel, "Your game board was sent to your DMs, " COLOR RED + username + RESET ".");
	BotResponseHandler::sendPlayerGrid(bot_socket_fd, username, grid);
}

void	BotResponseHandler::sendSolution( int bot_socket_fd, const std::string& username, const std::string& channel, const Grid& grid )
{
	if ( !channel.empty() )
		BotResponseHandler::sendResponse(bot_socket_fd, username, channel, "The solution to your single player game was sent to your DMs, " COLOR RED + username + RESET ".");
	BotResponseHandler::sendPlayerGrid(bot_socket_fd, username, grid, GridType::SOLUTION);
}

void	BotResponseHandler::sendNewGame( int bot_socket_fd, const std::string& username, const std::string& channel, const Grid& grid )
{
	if ( !channel.empty() )
		BotResponseHandler::sendResponse(bot_socket_fd, username, channel, "Your single player game was reset, " COLOR RED + username + RESET ". A new grid was sent to your DMs.");
	BotResponseHandler::sendPlayerGrid(bot_socket_fd, username, grid);
}

void BotResponseHandler::sendUnknownCmdFeedback( int bot_socket_fd, const std::string& username, const std::string& channel, const std::string& cmd )
{
	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, "Unknow command: " COLOR RED + cmd + RESET);
	sendResponse(bot_socket_fd, username, channel, "Please type " COLOR YELLOW "!help" RESET " for a list of commands.");
	sendResponse(bot_socket_fd, username, channel, "");
}

void BotResponseHandler::sendGameAlreadyRunningFeedback( int bot_socket_fd, const std::string& username, const std::string& channel )
{
	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, "You already have a Battleships game running!");
	sendResponse(bot_socket_fd, username, channel, "Type " COLOR YELLOW "!board" RESET " to see your current game board or " COLOR YELLOW "!newgame" RESET " to start a fresh game.");
	sendResponse(bot_socket_fd, username, channel, "");
}

void BotResponseHandler::sendNoGameFeedback( int bot_socket_fd, const std::string& username, const std::string& channel )
{
	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, "You need to start a game before you can use this command. You can start a new game by typing " COLOR YELLOW "!start" RESET ".");
	sendResponse(bot_socket_fd, username, channel, "");
}

void BotResponseHandler::sendNoTargetFeedback( int bot_socket_fd, const std::string& username, const std::string& channel )
{
	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, "Please specify a target (e.g. " COLOR YELLOW "!fire B3" RESET ")");
	sendResponse(bot_socket_fd, username, channel, "");
}

void BotResponseHandler::sendInvalidTargetFeedback( int bot_socket_fd, const std::string& username, const std::string& channel, const std::string& target )
{
	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, "This is not a valid target: " COLOR RED + capitalize(target) + RESET);
	sendResponse(bot_socket_fd, username, channel, "");
}

void BotResponseHandler::sendHelp( int bot_socket_fd, const std::string& username, const std::pair<std::string, std::string>& cmd )
{
	sendResponse(bot_socket_fd, username, "", cmd.first);
	sendResponse(bot_socket_fd, username, "", cmd.second);
	sendResponse(bot_socket_fd, username, "", "");
}

void BotResponseHandler::sendMissFeedback( int bot_socket_fd, const std::string& username, const std::string& target, const std::string& opponent )
{
	if (opponent.length())
	{
		sendResponse(bot_socket_fd, opponent, "", COLOR RED + username + "'s" RESET " shot at " COLOR YELLOW + capitalize(target) + COLOR CYAN " missed" RESET "!");
		sendResponse(bot_socket_fd, opponent, "", "");
	}

	sendResponse(bot_socket_fd, username, "", "Your shot at " COLOR YELLOW + capitalize(target) + COLOR CYAN " missed" RESET "!");
	sendResponse(bot_socket_fd, username, "", "");
}

void BotResponseHandler::sendHitFeedback( int bot_socket_fd, const std::string& username, const std::string& target, const std::string& opponent )
{
	if (opponent.length())
	{
		sendResponse(bot_socket_fd, opponent, "", COLOR RED + username + "'s" RESET " shot at " COLOR YELLOW + capitalize(target) + RESET " was a " COLOR RED "hit" RESET "!");
		sendResponse(bot_socket_fd, username, "", "You " COLOR RED "hit" RESET " one of " COLOR RED + opponent + "'s" RESET " ships at " COLOR YELLOW + capitalize(target) + RESET "!");
		sendResponse(bot_socket_fd, opponent, "", "");
		sendResponse(bot_socket_fd, username, "", "");
	}
	else
	{
		sendResponse(bot_socket_fd, username, "", "You " COLOR RED "hit" RESET " an enemy ship at " COLOR YELLOW + capitalize(target) + RESET "!");
		sendResponse(bot_socket_fd, username, "", "");
	}
}

void BotResponseHandler::sendSunkFeedback( int bot_socket_fd, const std::string& username, const std::string& sunkName, const std::string& opponent )
{
	if (opponent.length())
	{
		sendResponse(bot_socket_fd, opponent, "", BOLD COLOR RED + username + " sunk your " + sunkName + "!" RESET);
		sendResponse(bot_socket_fd, username, "", BOLD COLOR PURPLE "You sunk " + opponent + "'s " + sunkName + "! Congrats, keep going!" RESET);
		sendResponse(bot_socket_fd, opponent, "", "");
		sendResponse(bot_socket_fd, username, "", "");
	}
	else
	{
		sendResponse(bot_socket_fd, username, "", BOLD COLOR PURPLE "You sunk the enemy's " + sunkName + "! Congrats, keep going!" RESET);
		sendResponse(bot_socket_fd, username, "", "");
	}
}

void BotResponseHandler::sendWonFeedback( int bot_socket_fd, const std::string& username, const std::string& opponent )
{
	if (opponent.length())
	{
		sendResponse(bot_socket_fd, username, "", BOLD COLOR GREEN "You sunk all of " + opponent + "'s ships and won the game!" RESET);
		sendResponse(bot_socket_fd, username, "", "");
		sendResponse(bot_socket_fd, username, "", "To play against " COLOR RED + opponent + RESET " again type " COLOR YELLOW "!challenge " + opponent + RESET " to challenge them to another round.");
		sendResponse(bot_socket_fd, username, "", "");

		sendResponse(bot_socket_fd, opponent, "", BOLD COLOR RED "Oof, " + username + " sunk all of your ships. You have been defeated." RESET);
		sendResponse(bot_socket_fd, opponent, "", "");
		sendResponse(bot_socket_fd, opponent, "", "To challenge " COLOR RED + username + RESET " to a rematch type " COLOR YELLOW "!challenge " + username + RESET ". You'll get 'em this time!");
		sendResponse(bot_socket_fd, opponent, "", "");
	}
	else
	{
		sendResponse(bot_socket_fd, username, "", BOLD COLOR GREEN "Well done, you sunk all of the enemy's ships!" RESET);
		sendResponse(bot_socket_fd, username, "", "");
		sendResponse(bot_socket_fd, username, "", "To play again just type " COLOR YELLOW "!start" RESET " in the Battleships channel or as a private message to BattleShipsBot.");
		sendResponse(bot_socket_fd, username, "", "");
	}
}

void BotResponseHandler::sendWelcome( int bot_socket_fd, const std::string& username, const std::string& channel )
{
	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, BOLD COLOR LIGHT_CYAN "Hello, I am the BattleshipsBot, " + username + "!" RESET);
	sendResponse(bot_socket_fd, username, channel, BOLD COLOR LIGHT_CYAN "To start a single player game type " COLOR YELLOW "!start" COLOR LIGHT_CYAN " and " COLOR YELLOW "!fire <target>" COLOR LIGHT_CYAN " (e.g., !fire B3) to shoot at a target on the grid." RESET);
	sendResponse(bot_socket_fd, username, channel, BOLD COLOR LIGHT_CYAN "To challenge another user to a multiplayer match type " COLOR YELLOW "!challenge <username>" COLOR LIGHT_CYAN " (e.g., !challenge bob). To accept another users challenge type " COLOR YELLOW "!accept <username>" COLOR LIGHT_CYAN ". In multiplayer use the command " COLOR YELLOW "!shoot <username> <target>" COLOR LIGHT_CYAN " to take a shot at another player's fleet (e.g., !shoot bob A3)." RESET);
	sendResponse(bot_socket_fd, username, channel, BOLD COLOR LIGHT_CYAN "For a full list of commands type " COLOR YELLOW "!help" COLOR LIGHT_CYAN "." RESET);
	sendResponse(bot_socket_fd, username, channel, "");
}





/* ===================== Bot Response (Multiplayer specific) ===================== */

void	BotResponseHandler::sendChallenge( const Bot& bot, const std::string& challenger, const std::string& challenged, const std::string& channel )
{
	int	bot_socket_fd { bot.getSocket() };

	if ( !channel.empty() )
	{
		sendResponse(bot_socket_fd, "", channel, "");
		sendResponse(bot_socket_fd, "", channel, COLOR RED + challenger + RESET " challenges " COLOR RED + challenged + RESET " to a game of Battleships!");
		sendResponse(bot_socket_fd, "", channel, "To accept the challenge and start playing, " COLOR RED + challenged + RESET ", please type " COLOR YELLOW "!accept " + challenger + RESET ".");
		sendResponse(bot_socket_fd, "", channel, "");
	}

	if ( !bot.memberInChannel(challenger) || channel.empty())
	{
		sendResponse(bot_socket_fd, challenger, "", "");
		sendResponse(bot_socket_fd, challenger, "", "You have challenged " COLOR RED + challenged + RESET " to a game of Battleships!");
		sendResponse(bot_socket_fd, challenger, "", "They have been notified and will need to accept your challenge before the game can begin.");
		sendResponse(bot_socket_fd, challenger, "", "");
	}

	if ( !bot.memberInChannel(challenged) || channel.empty() )
	{
		sendResponse(bot_socket_fd, challenged, "", "");
		sendResponse(bot_socket_fd, challenged, "", "You have been challenged to a game of Battleships by " COLOR RED + challenger + RESET "!");
		sendResponse(bot_socket_fd, challenged, "", "To accept the challenge and start playing type " COLOR YELLOW "!accept " + challenger + RESET ".");
		sendResponse(bot_socket_fd, challenged, "", "");
	}
}

void	BotResponseHandler::sendNoChallengedFeedback( const Bot& bot, const std::string& username, const std::string& channel )
{
	int	bot_socket_fd { bot.getSocket() };

	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, "Please specify the name of the user you would like to challenge (e.g. " COLOR YELLOW "!challenge bob" RESET ").");
	sendResponse(bot_socket_fd, username, channel, "");
}

void	BotResponseHandler::sendAlreadyChallengedFeedback( const Bot& bot, const std::string& username, const std::string& channel, const std::string& challenged )
{
	int	bot_socket_fd { bot.getSocket() };

	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, "You have already challenged " COLOR RED + challenged + RESET " or they have already challenged you to a game of Battleships. Please either accept their challenge or wait for them to accept your challenge.");
	sendResponse(bot_socket_fd, username, channel, "");
}

void	BotResponseHandler::sendAccept( const Bot& bot, const std::string& challenger, const std::string& challenged, const std::string& channel )
{
	int	bot_socket_fd { bot.getSocket() };

	if ( !channel.empty() )
	{
		sendResponse(bot_socket_fd, "", channel, "");
		sendResponse(bot_socket_fd, "", channel, COLOR RED + challenged + RESET " has accepted " COLOR RED + challenger + RESET "'s challenge to a game of Battleships! The game will now begin.");
		sendResponse(bot_socket_fd, "", channel, "Your grids have been sent to your DMs.");
		sendResponse(bot_socket_fd, "", channel, "");
	}

	if ( !bot.memberInChannel(challenger) || channel.empty() )
	{
		sendResponse(bot_socket_fd, challenger, "", "");
		sendResponse(bot_socket_fd, challenger, "", COLOR RED + challenged + RESET " has accepted your challenge! The game will now begin.");
		sendResponse(bot_socket_fd, challenger, "", "");
	}

	if ( !bot.memberInChannel(challenged) || channel.empty() )
	{
		sendResponse(bot_socket_fd, challenged, "", "");
		sendResponse(bot_socket_fd, challenged, "", "You have accepted " COLOR RED + challenger + RESET "'s challenge! The game will now begin.");
		sendResponse(bot_socket_fd, challenged, "", "");
	}
}

void	BotResponseHandler::sendCannotChallengeSelfFeedback( const Bot& bot, const std::string& username, const std::string& channel )
{
	int	bot_socket_fd { bot.getSocket() };

	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, "You cannot challenge yourself to a game of Battleships...");
	sendResponse(bot_socket_fd, username, channel, "");
}

void	BotResponseHandler::sendCannotAcceptSelfFeedback( const Bot& bot, const std::string& username, const std::string& channel )
{
	int	bot_socket_fd { bot.getSocket() };

	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, "You cannot accept a challenge to a game of Battleships from yourself...");
	sendResponse(bot_socket_fd, username, channel, "");
}

void	BotResponseHandler::sendNoChallengerFeedback( const Bot& bot, const std::string& username, const std::string& channel )
{
	int	bot_socket_fd { bot.getSocket() };

	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, "Please specify the name of the user whom's challenge you would like to accept (e.g. " COLOR YELLOW "!accept alice" RESET ").");
	sendResponse(bot_socket_fd, username, channel, "");
}

void	BotResponseHandler::sendNoChallengeToAcceptFeedback( const Bot& bot, const std::string& username, const std::string& channel,const std::string& challenger )
{
	int	bot_socket_fd { bot.getSocket() };

	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, "The user " COLOR RED + challenger + RESET " has not challenged you to a game of Battleships yet.");
	sendResponse(bot_socket_fd, username, channel, "Please wait for them to send you a challenge before accepting.");
	sendResponse(bot_socket_fd, username, channel, "Alternatively, you can challenge them yourself by typing " COLOR YELLOW "!challenge " + challenger + RESET ".");
	sendResponse(bot_socket_fd, username, channel, "");
}

void	BotResponseHandler::sendMPGameAlreadyRunningFeedback( const Bot& bot, const std::string& username, const std::string& opponent, const std::string& channel )
{
	int	bot_socket_fd { bot.getSocket() };

	if ( !channel.empty() )
	{
		sendResponse(bot_socket_fd, "", channel, "");
		sendResponse(bot_socket_fd, "", channel, "There is already a Battleships game in progress between " COLOR RED + username + RESET " and " COLOR RED + opponent + RESET ".");
		sendResponse(bot_socket_fd, "", channel, COLOR RED + username + RESET ": to see your current game board type " COLOR YELLOW "!shots " + opponent + RESET ".");
		sendResponse(bot_socket_fd, "", channel, "");

		return;
	}

	sendResponse(bot_socket_fd, username, "", "");
	sendResponse(bot_socket_fd, username, "", "There is already a Battleships game in progress between you and " COLOR RED + opponent + RESET ".");
	sendResponse(bot_socket_fd, username, "", "Type " COLOR YELLOW "!shots " + opponent + RESET " to see your current game board.");
	sendResponse(bot_socket_fd, username, "", "");
}

void	BotResponseHandler::sendNoMPGameFeedback( const Bot& bot, const std::string& username, const std::string& opponent, const std::string& channel )
{
	int	bot_socket_fd { bot.getSocket() };

	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, "There is no Battleships game running between you and '" COLOR RED + opponent + RESET "' at the moment.");
	sendResponse(bot_socket_fd, username, channel, "You can challenge them to a game by typing " COLOR YELLOW "!challenge " + opponent + RESET ".");
	sendResponse(bot_socket_fd, username, channel, "");
}

void	BotResponseHandler::sendNotYourTurnFeedback( const Bot& bot, const std::string& username, const std::string& opponent, const std::string& channel )
{
	int	bot_socket_fd { bot.getSocket() };

	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, "It is not your turn! Please wait for " COLOR RED + opponent + RESET " to take their shot.");
	sendResponse(bot_socket_fd, username, channel, "");
}

void	BotResponseHandler::sendTurnInfo( const Bot& bot, const std::string& player_one, const std::string& player_two, const MPGame* mp_game )
{
	const std::string&	current_player { mp_game->getCurrentPlayer() };
	int					bot_socket_fd { bot.getSocket() };

	if ( player_one == current_player )
	{
		sendResponse(bot_socket_fd, player_one, "", "It is your turn, " COLOR RED + player_one + RESET "!");
		sendResponse(bot_socket_fd, player_one, "", "");

		sendResponse(bot_socket_fd, player_two, "", "It is " COLOR RED + player_one + "'s" RESET " turn.");
		sendResponse(bot_socket_fd, player_two, "", "");
	}
	else
	{
		sendResponse(bot_socket_fd, player_two, "", "It is your turn, " COLOR RED + player_two + RESET "!");
		sendResponse(bot_socket_fd, player_two, "", "");

		sendResponse(bot_socket_fd, player_one, "", "It is " COLOR RED + player_two + "'s" RESET " turn.");
		sendResponse(bot_socket_fd, player_one, "", "");
	}
}

void	BotResponseHandler::sendCannotSurrenderToSelfFeedback( const Bot& bot, const std::string& username, const std::string& channel )
{
	int	bot_socket_fd { bot.getSocket() };

	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, "You cannot surrender to yourself...");
	sendResponse(bot_socket_fd, username, channel, "");
}

void	BotResponseHandler::sendSurrender( const Bot& bot, const std::string& username, const std::string& opponent, const std::string& channel )
{
	int	bot_socket_fd { bot.getSocket() };

	if ( !channel.empty() )
	{
		sendResponse(bot_socket_fd, "", channel, "");
		sendResponse(bot_socket_fd, "", channel, COLOR RED + username + RESET " publicly surrenders to " COLOR RED + opponent + RESET ".");
		sendResponse(bot_socket_fd, "", channel, COLOR GREEN + opponent + " has won the match!" RESET);
		sendResponse(bot_socket_fd, "", channel, "");
	}

	if ( !bot.memberInChannel(username) || channel.empty() )
	{
		sendResponse(bot_socket_fd, username, "", "");
		sendResponse(bot_socket_fd, username, "", COLOR RED "You have surrendered your fleet to " + opponent + ". You lost the battle." RESET);
		sendResponse(bot_socket_fd, username, "", "");
	}

	if ( !bot.memberInChannel(opponent) || channel.empty() )
	{
		sendResponse(bot_socket_fd, opponent, "", "");
		sendResponse(bot_socket_fd, opponent, "", COLOR GREEN + username + " has surrendered! You have won the game!" RESET);
		sendResponse(bot_socket_fd, opponent, "", "");
	}
}

void	BotResponseHandler::sendCannotShootSelf( const Bot& bot, const std::string& username, const std::string& channel )
{
	int	bot_socket_fd { bot.getSocket() };

	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, "Hey, watch where you point that thing, the enemy is over there!");
	sendResponse(bot_socket_fd, username, channel, "");
}

void	BotResponseHandler::sendCannotShowFleetOrShotsAgainstSelf( const Bot& bot, const std::string& username, const std::string& channel )
{
	int	bot_socket_fd { bot.getSocket() };

	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, "Please specify the name of the user your fleet is fighting, not yourself.");
	sendResponse(bot_socket_fd, username, channel, "");
}

void	BotResponseHandler::sendShot( const Bot& bot, const std::string& username, const std::string& channel, const std::string& opponent, const Grid& playerShots, const Grid& opponentFleet )
{
	if ( !channel.empty() )
		BotResponseHandler::sendResponse(bot.getSocket(), "", channel, "You fired a shot at " + opponent + "'s fleet, " COLOR RED + username + RESET ". Check your DMs for the shot result.");
	BotResponseHandler::sendPlayerGrid(bot.getSocket(), username, playerShots, BotResponseHandler::GridType::TRACKING, opponent);
	BotResponseHandler::sendPlayerGrid(bot.getSocket(), opponent, opponentFleet, BotResponseHandler::GridType::REFERENCE, username);
}

void	BotResponseHandler::sendFleet( const Bot& bot, const std::string& username, const std::string& channel, const std::string& opponent, const Grid& grid )
{
	int	bot_socket_fd { bot.getSocket() };

	if ( !channel.empty() )
		BotResponseHandler::sendResponse(bot_socket_fd, username, channel, "A grid with your fleet was sent to your DMs, " COLOR RED + username + RESET ".");
	BotResponseHandler::sendPlayerGrid(bot_socket_fd, username, grid, BotResponseHandler::GridType::REFERENCE, opponent);
}

void	BotResponseHandler::sendShots( const Bot& bot, const std::string& username, const std::string& channel, const std::string& opponent, const Grid& grid )
{
	int	bot_socket_fd { bot.getSocket() };

	if ( !channel.empty() )
		BotResponseHandler::sendResponse(bot_socket_fd, username, channel, "Your shots grid was sent to your DMs, " COLOR RED + username + RESET ".");
	BotResponseHandler::sendPlayerGrid(bot_socket_fd, username, grid, BotResponseHandler::GridType::TRACKING, opponent);
}





/* ===================== Helpers ===================== */

void	BotResponseHandler::sendResponse( int bot_socket_fd, const std::string& username, const std::string& channel, const std::string& msg )
{
	std::string	prefix {};
	std::string	response {};

	if (channel.length())
		prefix = "PRIVMSG " +  channel + " :";
	else
		prefix = "PRIVMSG " +  username + " :";

	response = prefix + msg + "\r\n";

	send(bot_socket_fd, response.data(), response.length(), 0);
}

std::string	BotResponseHandler::capitalize( const std::string& target )
{
	std::string capTarget {};

	capTarget = target;
	capTarget[0] = std::toupper(capTarget[0]);

	return capTarget;
}
