/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BotCommands.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 17:06:14 by dlippelt          #+#    #+#             */
/*   Updated: 2025/12/04 16:54:29 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <map>

#include "Game.hpp"
#include "Bot.hpp"
#include "BotResponseHandler.hpp"

class Bot;

class BotCommands
{
	public:
		BotCommands() = delete;

		enum BotCommandType
		{
			CMD_START,
			CMD_FIRE,
			CMD_SOLUTION,
			CMD_BOARD,
			CMD_NEWGAME,
			CMD_HELP,
			CMD_CHALLENGE,
			CMD_ACCEPT,
			CMD_UNKNOWN,
			CMD_NOTACMD
		};

		static inline const std::map<std::string, BotCommandType> k_commands
		{
			{"!start", CMD_START},
			{"!fire", CMD_FIRE},
			{"!solution", CMD_SOLUTION},
			{"!board", CMD_BOARD},
			{"!newgame", CMD_NEWGAME},
			{"!help", CMD_HELP},
			{"!challenge", CMD_CHALLENGE},
			{"!accept", CMD_ACCEPT},
		};

		static inline const std::vector<std::pair<std::string, std::string>> k_help_content
		{
			{COLOR YELLOW "!help" RESET, "Show the current help menu."},
			{COLOR YELLOW "!start" RESET, "Start playing battlehips."},
			{COLOR YELLOW "!newgame" RESET, "Start a fresh game (will abort your currently running game)."},
			{COLOR YELLOW "!fire <target>" RESET, "Fire a shot at the board (e.g. '!fire B3')."},
			{COLOR YELLOW "!board" RESET, "Show your current game board."},
			{COLOR YELLOW "!solution" RESET, "Show the board with all ships visible (i.e. cheat sheet)."},
			{COLOR YELLOW "!challenge <username>" RESET, "Challenge another user to a game of battleships (e.g. '!challenge bob). Please make sure to enter the username correctly or they will not receive the challenge!"},
			{COLOR YELLOW "!accept <username>" RESET, "Accept another user's challenge to a game of battleships (e.g. '!accept alice). Please make sure to enter the username correctly or the challenge will not be accepted!"},
		};

		static void	executeCommand( const std::string& username, const std::string& channel, const std::string& message, Bot& bot );

	private:
		static void	startGame( const std::string& username, const std::string& channel, Bot& bot );
		static void	fireShot( const std::string& username, const std::string& channel, const std::string& msg, Bot& bot );
		static void	showSolution( const std::string& username, const std::string& channel, const Bot& bot );
		static void	showBoard( const std::string& username, const std::string& channel, const Bot& bot );
		static void	newGame( const std::string& username, const std::string& channel, Bot& bot );
		static void help( const std::string& username, const std::string& channel, const Bot& bot );

		// MP Game commands
		static void	challenge( const std::string& challenger, const std::string& channel, const std::string& msg, Bot& bot );
		static void	acceptChallenge( const std::string& challenged, const std::string& channel, const std::string& msg, Bot& bot );

		static void startMPGame( const std::string& challenger, const std::string& challenged, const std::string& channel, Bot& bot );

		static BotCommandType	getCmdType( const std::string& command );
		static bool			challengeExists( const std::string& challenger, const std::string& challenged, const Bot& bot );
};
