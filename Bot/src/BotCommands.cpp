/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BotCommands.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 17:08:37 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/25 17:56:15 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BotCommands.hpp"

/* ===================== Bot Commands ===================== */

void	BotCommands::executeCommand( const std::string& username, const std::string& channel, const std::string& message, Bot& bot )
{
	std::string target {};
	std::string cmd { message.substr(0, message.find_first_of(" ")) };

	switch ( getCmdType(cmd) )
	{
	case CMD_START:
		if (message.length() == e_start)
			startGame(username, channel, bot);
		else
			BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "To start playing please type only '!start'");
		break;
	case CMD_FIRE:
		target = message.substr(message.find_first_of(" ") + 1);
		if (message.length() == e_fire)
			fireShot(username, channel, target, bot);
		else
			BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "This is not a valid battleships target: '" + capitalize(target) + "'");
		break;
	case CMD_BOARD:
		if (message.length() == e_board)
			showBoard(username, channel, bot);
		else
			BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "To see the current game's grid please type only '!board'");
		break;
	case CMD_SOLUTION:
		if (message.length() == e_solution)
			showSolution(username, channel, bot);
		else
			BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "To see the current game's solution please type only '!solution'");
		break;
	case CMD_NEWGAME:
		if (message.length() == e_newgame)
			newGame(username, channel, bot);
		else
			BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "To start a fresh game please type only '!newgame'");
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
			BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "You already have a Battleships game running!");
			BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "Type '!board' to see your current game board or '!newgame' to start a fresh game.");
			return;
		}
	}
	catch ( const std::exception& e )
	{
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, e.what());
		return;
	}

	BotResponseHandler::sendGrid(bot.getSocket(), username, channel, "Battleships grid for " + username + "'s game", game->getPlayerGridObject());
}

void	BotCommands::fireShot( const std::string& username, const std::string& channel, const std::string& target, Bot& bot )
{
	const std::map<std::string, Game*>& games { bot.getGames() };

	auto it = games.find(username);
	if (it == games.end())
	{
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "You need to start a game before you can use this command. You can start a new game by typing '!start'");
		return;
	}

	Game* game { it->second };

	if (!game->validInput(target))
	{
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "This is not a valid battleships target: '" + capitalize(target) + "'");
		return;
	}

	ShotResult sr { game->processShot(target) };

	BotResponseHandler::sendGrid(bot.getSocket(), username, channel, "Battleships grid for " + username + "'s game", game->getPlayerGridObject());

	switch (sr)
	{
	case ShotResult::MISS:
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "Your shot at " + capitalize(target) + " missed!");
		break;
	case ShotResult::HIT:
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "You hit an enemy ship at " + capitalize(target) + "!");
		break;
	case ShotResult::SUNK:
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "You sunk an enemy ship! Congrats, keep going!");
		break;
	case ShotResult::WON:
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "Well done, you sunk all of the enemy's ships!");
		bot.removeGame(username);
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "To play again just type !start in the Battleships channel or as a private message to BattleShipsBot.");
		break;
	default:
		break;
	}
}

void	BotCommands::showBoard( const std::string& username, const std::string& channel, const Bot& bot )
{
	const std::map<std::string, Game*>& games { bot.getGames() };

	auto it = games.find(username);
	if (it == games.end())
	{
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "You need to start a game before you can use this command. You can start a new game by typing '!start'");
		return;
	}

	Game* game { it->second };

	BotResponseHandler::sendGrid(bot.getSocket(), username, channel, "Battleships grid for " + username + "'s game", game->getPlayerGridObject());
}

void	BotCommands::showSolution( const std::string& username, const std::string& channel, const Bot& bot )
{
	const std::map<std::string, Game*>& games { bot.getGames() };

	auto it = games.find(username);
	if (it == games.end())
	{
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "You need to start a game before you can use this command. You can start a new game by typing '!start'");
		return;
	}

	Game* game { it->second };

	BotResponseHandler::sendGrid(bot.getSocket(), username, channel, "Battleships game solution for " + username + "'s game", game->getGridObject());
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

	BotResponseHandler::sendGrid(bot.getSocket(), username, channel, "Battleships grid for " + username + "'s game", game->getPlayerGridObject());
}





/* ===================== Utility ===================== */

BotCommands::CommandType BotCommands::getCmdType( const std::string& command )
{
	auto it { k_commands.find(command) };

	return ( it != k_commands.end() ? it->second : CMD_UNKNOWN );
}

std::string	BotCommands::capitalize( const std::string& target )
{
	std::string capTarget {};

	capTarget = target;
	capTarget[0] = std::toupper(capTarget[0]);

	return capTarget;
}
