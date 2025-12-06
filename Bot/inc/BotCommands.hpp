/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BotCommands.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 17:06:14 by dlippelt          #+#    #+#             */
/*   Updated: 2025/12/06 11:02:36 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <map>

#include "Game.hpp"
#include "MPGame.hpp"
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
			CMD_SHOOT,
			CMD_SURRENDER,
			CMD_FLEET,
			CMD_SHOTS,
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
			{"!shoot", CMD_SHOOT},
			{"!surrender", CMD_SURRENDER},
			{"!fleet", CMD_FLEET},
			{"!shots", CMD_SHOTS},
		};

		static inline const std::vector<std::pair<std::string, std::string>> k_help_content
		{
			{COLOR YELLOW "!help" RESET, "Show the current help menu."},
			{COLOR YELLOW "!start" RESET, "Start playing a single player battlehips."},
			{COLOR YELLOW "!newgame" RESET, "Start a fresh single player game (will abort your currently running game)."},
			{COLOR YELLOW "!fire <target>" RESET, "Fire a shot at the board in single player (e.g. '!fire B3')."},
			{COLOR YELLOW "!board" RESET, "Show your current game board in single player."},
			{COLOR YELLOW "!solution" RESET, "Show the single player board with all ships visible (i.e. cheat sheet)."},
			{COLOR YELLOW "!challenge <username>" RESET, "Challenge another user to a game of battleships (e.g. '!challenge bob). Please make sure to enter the username correctly or they will not receive the challenge!"},
			{COLOR YELLOW "!accept <username>" RESET, "Accept another user's challenge to a game of battleships (e.g. '!accept alice)."},
			{COLOR YELLOW "!shoot <username> <target>" RESET, "Take a shot at another user's board (e.g. '!shoot bob H4')"},
			{COLOR YELLOW "!surrender <username>" RESET, "Surrender to another player. This will immediately end your game with them (e.g. '!surrender bob' to surrender to bob)."},
			{COLOR YELLOW "!fleet <username>" RESET, "Show your current fleet against another player (e.g. '!fleet bob'). Will be visible to only you."},
			{COLOR YELLOW "!shots <username>" RESET, "Show your current shots grid against another player (e.g. '!shots bob'). Will be visible to only you."},
		};

		static void	executeCommand( const std::string& username, const std::string& channel, const std::string& message, Bot& bot );

	private:
		static void	start( const std::string& username, const std::string& channel, Bot& bot );
		static void	fire( const std::string& username, const std::string& channel, const std::string& msg, Bot& bot );
		static void	solution( const std::string& username, const std::string& channel, const Bot& bot );
		static void	board( const std::string& username, const std::string& channel, const Bot& bot );
		static void	newGame( const std::string& username, const std::string& channel, Bot& bot );
		static void help( const std::string& username, const std::string& channel, const Bot& bot );

		// MP specific Game commands
		static void	challenge( const std::string& challenger, const std::string& channel, const std::string& msg, Bot& bot );
		static void	acceptChallenge( const std::string& challenged, const std::string& channel, const std::string& msg, Bot& bot );
		static void shoot( const std::string& username, const std::string& channel, const std::string& msg, Bot& bot );
		static void surrender( const std::string& username, const std::string& channel, const std::string& msg, Bot& bot );
		static void fleet( const std::string& username, const std::string& channel, const std::string& msg, Bot& bot );
		static void shots( const std::string& username, const std::string& channel, const std::string& msg, Bot& bot );


		static BotCommandType	getCmdType( const std::string& command );
		static void				startMPGame( const std::string& challenger, const std::string& challenged, const std::string& channel, Bot& bot );
		static bool				challengeExists( const std::string& challenger, const std::string& challenged, const Bot& bot );
		static bool				gameAlreadyExists( const std::string& challenger, const std::string& challenged, const Bot& bot );
		static std::string		getOpponentName( const std::string& msg );
		static std::string		getMPTarget( const std::string& msg );
};
