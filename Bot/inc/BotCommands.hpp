/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BotCommands.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 17:06:14 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/25 17:56:52 by dlippelt         ###   ########.fr       */
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

		enum CommandLengths
		{
			e_start = 6,
			e_fire = 8,
			e_solution = 9,
			e_board = 6,
			e_newgame = 8
		};

		enum CommandType
		{
			CMD_START,
			CMD_FIRE,
			CMD_SOLUTION,
			CMD_BOARD,
			CMD_NEWGAME,
			CMD_UNKNOWN
		};

		static inline const std::map<std::string, CommandType> k_commands
		{
			{"!start", CMD_START},
			{"!fire", CMD_FIRE},
			{"!solution", CMD_SOLUTION},
			{"!board", CMD_BOARD},
			{"!newgame", CMD_NEWGAME}
		};


		static void	executeCommand( const std::string& username, const std::string& channel, const std::string& message, Bot& bot );

	private:
		static void	startGame( const std::string& username, const std::string& channel, Bot& bot );
		static void	fireShot( const std::string& username, const std::string& channel, const std::string& target, Bot& bot );
		static void	showSolution( const std::string& username, const std::string& channel, const Bot& bot );
		static void	showBoard( const std::string& username, const std::string& channel, const Bot& bot );
		static void	newGame( const std::string& username, const std::string& channel, Bot& bot );


		static CommandType	getCmdType( const std::string& command );
		static std::string	capitalize( const std::string& target );
};
