/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BotCommands.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 17:06:14 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/26 12:31:43 by dlippelt         ###   ########.fr       */
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

		enum CommandType
		{
			CMD_START,
			CMD_FIRE,
			CMD_SOLUTION,
			CMD_BOARD,
			CMD_NEWGAME,
			CMD_HELP,
			CMD_UNKNOWN,
			CMD_NOTACMD
		};

		static inline const std::map<std::string, CommandType> k_commands
		{
			{"!start", CMD_START},
			{"!fire", CMD_FIRE},
			{"!solution", CMD_SOLUTION},
			{"!board", CMD_BOARD},
			{"!newgame", CMD_NEWGAME},
			{"!help", CMD_HELP}
		};

		static inline const std::vector<std::pair<std::string, std::string>> k_help_content
		{
			{"\x03" "08" "!help" "\x03", "Show the current help menu."},
			{"\x03" "08" "!start" "\x03", "Start playing battlehips."},
			{"\x03" "08" "!newgame" "\x03", "Start a fresh game (will abort your currently running game)."},
			{"\x03" "08" "!fire <target>" "\x03", "Fire a shot at the board (e.g. '!fire B3')."},
			{"\x03" "08" "!board" "\x03", "Show your current game board."},
			{"\x03" "08" "!solution" "\x03", "Show the board with all ships visible (i.e. cheat sheet)."}
		};

		static void	executeCommand( const std::string& username, const std::string& channel, const std::string& message, Bot& bot );

	private:
		static void	startGame( const std::string& username, const std::string& channel, Bot& bot );
		static void	fireShot( const std::string& username, const std::string& channel, const std::string& msg, Bot& bot );
		static void	showSolution( const std::string& username, const std::string& channel, const Bot& bot );
		static void	showBoard( const std::string& username, const std::string& channel, const Bot& bot );
		static void	newGame( const std::string& username, const std::string& channel, Bot& bot );
		static void help( const std::string& username, const std::string& channel, Bot& bot );


		static CommandType	getCmdType( const std::string& command );
		static std::string	capitalize( const std::string& target );
};
