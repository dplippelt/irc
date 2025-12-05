/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BotResponseHandler.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 17:32:25 by dlippelt          #+#    #+#             */
/*   Updated: 2025/12/05 10:28:56 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BotResponseHandler.hpp"

/* ===================== Bot Response ===================== */

void	BotResponseHandler::sendPlayerGrid( int bot_socket_fd, const std::string& username, const std::string& channel, const Grid& grid )
{
	std::string 		gridMsg { grid.getGridMsg() };
	std::istringstream	iss { gridMsg };
	std::string			line {};

	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, "Battleships grid for " COLOR RED + username + "'s " RESET "game");
	sendResponse(bot_socket_fd, username, channel, "");
	while ( std::getline(iss, line) )
		sendResponse(bot_socket_fd, username, channel, line);
	sendResponse(bot_socket_fd, username, channel, "");
}

void	BotResponseHandler::sendSolution( int bot_socket_fd, const std::string& username, const std::string& channel, const Grid& grid )
{
	std::string 		gridMsg { grid.getGridMsg() };
	std::istringstream	iss { gridMsg };
	std::string			line {};

	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, "Battleships game solution for " COLOR RED + username + "'s " RESET "game");
	sendResponse(bot_socket_fd, username, channel, "");
	while ( std::getline(iss, line) )
		sendResponse(bot_socket_fd, username, channel, line);
	sendResponse(bot_socket_fd, username, channel, "");
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

void BotResponseHandler::sendHelp( int bot_socket_fd, const std::string& username, const std::string& channel, const std::pair<std::string, std::string>& cmd )
{
	sendResponse(bot_socket_fd, username, channel, cmd.first);
	sendResponse(bot_socket_fd, username, channel, cmd.second);
	sendResponse(bot_socket_fd, username, channel, "");
}

void BotResponseHandler::sendMissFeedback( int bot_socket_fd, const std::string& username, const std::string& channel, const std::string& target )
{
	sendResponse(bot_socket_fd, username, channel, "Your shot at " COLOR YELLOW + capitalize(target) + COLOR CYAN " missed" RESET "!");
	sendResponse(bot_socket_fd, username, channel, "");
}

void BotResponseHandler::sendHitFeedback( int bot_socket_fd, const std::string& username, const std::string& channel, const std::string& target )
{
	sendResponse(bot_socket_fd, username, channel, "You " COLOR RED "hit" RESET " an enemy ship at " COLOR YELLOW + capitalize(target) + RESET "!");
	sendResponse(bot_socket_fd, username, channel, "");
}

void BotResponseHandler::sendSunkFeedback( int bot_socket_fd, const std::string& username, const std::string& channel, const std::string& sunkName )
{
	sendResponse(bot_socket_fd, username, channel, BOLD COLOR PURPLE "You sunk the enemy's " + sunkName + "! Congrats, keep going!" RESET);
	sendResponse(bot_socket_fd, username, channel, "");
}

void BotResponseHandler::sendWonFeedback( int bot_socket_fd, const std::string& username, const std::string& channel )
{
	sendResponse(bot_socket_fd, username, channel, BOLD COLOR GREEN "Well done, you sunk all of the enemy's ships!" RESET);
	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, "To play again just type " COLOR YELLOW "!start" RESET " in the Battleships channel or as a private message to BattleShipsBot.");
	sendResponse(bot_socket_fd, username, channel, "");
}

void BotResponseHandler::sendWelcome( int bot_socket_fd, const std::string& username, const std::string& channel )
{
	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, BOLD COLOR LIGHT_CYAN "Hello, I am the BattleshipsBot, " + username + "!" RESET);
	sendResponse(bot_socket_fd, username, channel, BOLD COLOR LIGHT_CYAN "Type " COLOR YELLOW "!start" COLOR LIGHT_CYAN " to start a game of battleships and " COLOR YELLOW "!fire <target>" COLOR LIGHT_CYAN " (e.g., !fire B3) to shoot at a target on the grid." RESET);
	sendResponse(bot_socket_fd, username, channel, BOLD COLOR LIGHT_CYAN "For a full list of commands type " COLOR YELLOW "!help" COLOR LIGHT_CYAN "." RESET);
	sendResponse(bot_socket_fd, username, channel, "");
}





/* ===================== Bot Response (Multiplayer) ===================== */

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

	if ( !bot.memberInChannel(challenger) )
	{
		sendResponse(bot_socket_fd, challenger, "", "");
		sendResponse(bot_socket_fd, challenger, "", "You have challenged " COLOR RED + challenged + RESET " to a game of Battleships!");
		sendResponse(bot_socket_fd, challenger, "", "They have been notified and will need to accept your challenge before the game can begin.");
		sendResponse(bot_socket_fd, challenger, "", "");
	}

	if ( !bot.memberInChannel(challenged) )
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

void	BotResponseHandler::sendAlreadyChallengedFeedback( const Bot& bot, const std::string& username, const std::string& channel,const std::string& challenged )
{
	int	bot_socket_fd { bot.getSocket() };

	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, "You have already challenged " COLOR RED + challenged + RESET " to a game of Battleships. Please wait for them to accept your challenge.");
	sendResponse(bot_socket_fd, username, channel, "");
}

void	BotResponseHandler::sendAccept( const Bot& bot, const std::string& challenger, const std::string& challenged, const std::string& channel )
{
	int	bot_socket_fd { bot.getSocket() };

	if ( !channel.empty() )
	{
		sendResponse(bot_socket_fd, "", channel, "");
		sendResponse(bot_socket_fd, "", channel, COLOR RED + challenged + RESET " has accepted " COLOR RED + challenger + RESET "'s challenge to a game of Battleships! The game will now begin.");
		sendResponse(bot_socket_fd, "", channel, "");
	}

	if ( !bot.memberInChannel(challenger) )
	{
		sendResponse(bot_socket_fd, challenger, "", "");
		sendResponse(bot_socket_fd, challenger, "", COLOR RED + challenged + RESET " has accepted your challenge! The game will now begin.");
		sendResponse(bot_socket_fd, challenger, "", "");
	}

	if ( !bot.memberInChannel(challenged) )
	{
		sendResponse(bot_socket_fd, challenged, "", "");
		sendResponse(bot_socket_fd, challenged, "", "You have accepted " COLOR RED + challenger + RESET "'s challenge! The game will now begin.");
		sendResponse(bot_socket_fd, challenged, "", "");
	}
}

void BotResponseHandler::sendNoChallengerFeedback( const Bot& bot, const std::string& username, const std::string& channel )
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

void	BotResponseHandler::sendMPGameAlreadyRunningFeedback( const Bot& bot, const std::string& challenger, const std::string& challenged, const std::string& channel )
{
	int	bot_socket_fd { bot.getSocket() };

	if ( !channel.empty() )
	{
		sendResponse(bot_socket_fd, "", channel, "");
		sendResponse(bot_socket_fd, "", channel, "There is already a Battleships game in progress between " COLOR RED + challenger + RESET " and " COLOR RED + challenged + RESET ".");
		sendResponse(bot_socket_fd, "", channel, COLOR RED + challenger + RESET ": to see your current game board type " COLOR YELLOW "!board " + challenged + RESET ".");
		sendResponse(bot_socket_fd, "", channel, COLOR RED + challenged + RESET ": to see your current game board type " COLOR YELLOW "!board " + challenger + RESET ".");
		sendResponse(bot_socket_fd, "", channel, "");
	}

	if ( !bot.memberInChannel(challenger) )
	{
		sendResponse(bot_socket_fd, challenger, "", "");
		sendResponse(bot_socket_fd, challenger, "", "There is already a Battleships game in progress between you and " COLOR RED + challenged + RESET ".");
		sendResponse(bot_socket_fd, challenger, "", "Type " COLOR YELLOW "!board " + challenged + RESET " to see your current game board.");
		sendResponse(bot_socket_fd, challenger, "", "");
	}

	if ( !bot.memberInChannel(challenged) )
	{
		sendResponse(bot_socket_fd, challenged, "", "");
		sendResponse(bot_socket_fd, challenged, "", "There is already a Battleships game in progress between you and " COLOR RED + challenger + RESET ".");
		sendResponse(bot_socket_fd, challenged, "", "Type " COLOR YELLOW "!board " + challenged + RESET " to see your current game board.");
		sendResponse(bot_socket_fd, challenged, "", "");
	}
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
