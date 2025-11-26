/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BotResponseHandler.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 17:32:25 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/26 14:49:03 by dlippelt         ###   ########.fr       */
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

void BotResponseHandler::sendSunkFeedback( int bot_socket_fd, const std::string& username, const std::string& channel )
{
	sendResponse(bot_socket_fd, username, channel, BOLD COLOR PURPLE "You sunk an enemy ship! Congrats, keep going!" RESET);
	sendResponse(bot_socket_fd, username, channel, "");
}

void BotResponseHandler::sendWonFeedback( int bot_socket_fd, const std::string& username, const std::string& channel )
{
	sendResponse(bot_socket_fd, username, channel, BOLD COLOR GREEN "Well done, you sunk all of the enemy's ships!" RESET);
	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, "To play again just type " COLOR YELLOW "!start" RESET " in the Battleships channel or as a private message to BattleShipsBot.");
	sendResponse(bot_socket_fd, username, channel, "");
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
