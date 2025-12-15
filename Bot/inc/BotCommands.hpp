/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BotCommands.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 17:06:14 by dlippelt          #+#    #+#             */
/*   Updated: 2025/12/15 18:14:34 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <map>
#include <memory>
#include "Game.hpp"
#include "MPGame.hpp"
#include "Bot.hpp"
#include "BotResponseHandler.hpp"

class Bot;
class BotResponseHandler;

class BotCommands
{
	public:
		~BotCommands();
		BotCommands() = delete;
		BotCommands( Bot& bot, const std::string& username, const std::string& channel, const std::string& message );
		BotCommands( const BotCommands& ) = delete;
		BotCommands& operator=( const BotCommands& ) = delete;

		Bot&								m_bot;
		std::unique_ptr<BotResponseHandler>	m_responseHandler;
		const std::string&					m_username;
		const std::string&					m_channel;
		const std::string&					m_message;

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
			CMD_FILES,
			CMD_FILE,
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
			{"!files", CMD_FILES},
			{"!file", CMD_FILE},
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
			{COLOR YELLOW "!files" RESET, "List available files for download."},
			{COLOR YELLOW "!file <filename>" RESET, "Download a specific file (e.g. '!file rules.txt')."},
		};

		void	executeCommand();

	private:
		void	start();
		void	fire();
		void	solution();
		void	board();
		void	newGame();
		void	help();

		// MP specific Game commands
		void	challenge( const std::string& challenger );
		void	acceptChallenge( const std::string& challenged );
		void	shoot();
		void	surrender();
		void	fleet();
		void	shots();

		// File transfer commands
		void	files();
		void	file();

		BotCommandType	getCmdType( const std::string& command );
		void			startMPGame( const std::string& challenger, const std::string& challenged );
		bool			challengeExists( const std::string& challenger, const std::string& challenged );
		bool			gameAlreadyExists( const std::string& challenger, const std::string& challenged );
		std::string		getOpponentName();
		std::string		getMPTarget();
};
