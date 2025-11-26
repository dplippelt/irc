/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BotCommands.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 17:08:37 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/26 13:57:51 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BotCommands.hpp"

/* ===================== Bot Commands ===================== */

void	BotCommands::executeCommand( const std::string& username, const std::string& channel, const std::string& message, Bot& bot )
{
	std::string cmd { message.substr(0, message.find_first_of(" ")) };

	switch ( getCmdType(cmd) )
	{
	case CMD_START:
		startGame(username, channel, bot);
		break;
	case CMD_FIRE:
		fireShot(username, channel, message, bot);
		break;
	case CMD_BOARD:
		showBoard(username, channel, bot);
		break;
	case CMD_SOLUTION:
		showSolution(username, channel, bot);
		break;
	case CMD_NEWGAME:
		newGame(username, channel, bot);
		break;
	case CMD_HELP:
		help(username, channel, bot);
		break;
	case CMD_UNKNOWN:
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "");
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "Unknow command: " "\x03" "04" + cmd + "\x03");
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "Please type " "\x03" "08" "!help" "\x03" " for a list of commands.");
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "");
		break;
	default:
		break;
	}
}

void	BotCommands::startGame( const std::string& username, const std::string& channel, Bot& bot )
{
	Game*	game;

	try
	{
		const std::map<std::string, Game*>& games { bot.getGames() };

		auto it = games.find(username);

		if ( it == games.end() )
		{
			game = new Game {};
			bot.addGame(username, game);
		}
		else
		{
			BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "");
			BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "You already have a Battleships game running!");
			BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "Type " "\x03" "08" "!board" "\x03" " to see your current game board or " "\x03" "08" "!newgame" "\x03" " to start a fresh game.");
			BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "");
			return;
		}
	}
	catch ( const std::exception& e )
	{
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, e.what());
		return;
	}

	BotResponseHandler::sendGrid(bot.getSocket(), username, channel, "Battleships grid for " "\x03" "04" + username + "'s " "\x03" "game", game->getPlayerGridObject());
}

void	BotCommands::fireShot( const std::string& username, const std::string& channel, const std::string& msg, Bot& bot )
{
	const std::map<std::string, Game*>& games { bot.getGames() };

	auto it = games.find(username);
	if (it == games.end())
	{
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "");
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "You need to start a game before you can use this command. You can start a new game by typing " "\x03" "08" "!start" "\x03");
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "");
		return;
	}

	std::size_t space_idx = msg.find_first_of(" ");
	if (space_idx == std::string::npos)
	{
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "");
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "Please specify a target (e.g. " "\x03" "08" "!fire B3" "\x03" ")");
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "");
		return;
	}

	Game* game { it->second };
	std::string target { msg.substr(space_idx + 1) };

	if (!game->validInput(target))
	{
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "");
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "This is not a valid target: " "\x03" "04" + capitalize(target) + "\x03");
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "");
		return;
	}

	ShotResult sr { game->processShot(target) };

	BotResponseHandler::sendGrid(bot.getSocket(), username, channel, "Battleships grid for " "\x03" "04" + username + "'s " "\x03" "game", game->getPlayerGridObject());

	switch (sr)
	{
	case ShotResult::MISS:
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "Your shot at " "\x03" "08" + capitalize(target) + "\x03" "10" " missed" "\x03" "!");
		break;
	case ShotResult::HIT:
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "You ""\x03" "04" "hit" "\x03" " an enemy ship at " "\x03" "08" + capitalize(target) + "\x03" "!");
		break;
	case ShotResult::SUNK:
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "\x02\x03" "06" "You sunk an enemy ship! Congrats, keep going!" "\x02\x03");
		break;
	case ShotResult::WON:
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "\x02\x03" "03" "Well done, you sunk all of the enemy's ships!" "\x02\x03");
		bot.removeGame(username);
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "");
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "To play again just type " "\x03" "08" "!start" "\x03" " in the Battleships channel or as a private message to BattleShipsBot.");
		break;
	default:
		break;
	}
	
	BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "");
}

void	BotCommands::showBoard( const std::string& username, const std::string& channel, const Bot& bot )
{
	const std::map<std::string, Game*>& games { bot.getGames() };

	auto it = games.find(username);
	if (it == games.end())
	{
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "You need to start a game before you can use this command. You can start a new game by typing " "\x03" "08" "!start" "\x03");
		return;
	}

	Game* game { it->second };

	BotResponseHandler::sendGrid(bot.getSocket(), username, channel, "Battleships grid for " "\x03" "04" + username + "'s " "\x03" "game", game->getPlayerGridObject());
}

void	BotCommands::showSolution( const std::string& username, const std::string& channel, const Bot& bot )
{
	const std::map<std::string, Game*>& games { bot.getGames() };

	auto it = games.find(username);
	if (it == games.end())
	{
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "You need to start a game before you can use this command. You can start a new game by typing " "\x03" "08" "!start" "\x03");
		return;
	}

	Game* game { it->second };

	BotResponseHandler::sendGrid(bot.getSocket(), username, channel, "Battleships game solution for " "\x03" "04" + username + "'s " "\x03" "game", game->getGridObject());
}

void	BotCommands::newGame( const std::string& username, const std::string& channel, Bot& bot )
{
	Game*	game;

	try
	{
		const std::map<std::string, Game*>& games { bot.getGames() };

		auto it = games.find(username);

		if ( it == games.end() )
		{
			game = new Game {};
			bot.addGame(username, game);
		}
		else
		{
			bot.removeGame(username);
			game = new Game {};
			bot.addGame(username, game);
		}
	}
	catch ( const std::exception& e )
	{
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, e.what());
		return;
	}

	BotResponseHandler::sendGrid(bot.getSocket(), username, channel, "Battleships grid for " "\x03" "04" + username + "'s " "\x03" "game", game->getPlayerGridObject());
}

void BotCommands::help( const std::string& username, const std::string& channel, Bot& bot )
{
	for ( const auto& cmd : k_help_content )
	{
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, cmd.first);
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, cmd.second);
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "");
	}
}





/* ===================== Utility ===================== */

BotCommands::CommandType BotCommands::getCmdType( const std::string& command )
{
	auto it { k_commands.find(command) };

	if (it != k_commands.end())
		return it->second;
	if (command[0] == '!')
		return CMD_UNKNOWN;
	return CMD_NOTACMD;
}

std::string	BotCommands::capitalize( const std::string& target )
{
	std::string capTarget {};

	capTarget = target;
	capTarget[0] = std::toupper(capTarget[0]);

	return capTarget;
}
