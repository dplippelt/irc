/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BotCommands.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 17:08:37 by dlippelt          #+#    #+#             */
/*   Updated: 2025/12/05 11:22:09 by dlippelt         ###   ########.fr       */
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
	case CMD_CHALLENGE:
		challenge(username, channel, message, bot);
		break;
	case CMD_ACCEPT:
		acceptChallenge(username, channel, message, bot);
		break;
	case CMD_UNKNOWN:
		BotResponseHandler::sendUnknownCmdFeedback(bot.getSocket(), username, channel, cmd);
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
			BotResponseHandler::sendGameAlreadyRunningFeedback(bot.getSocket(), username, channel);
			return;
		}
	}
	catch ( const std::exception& e )
	{
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, e.what());
		return;
	}

	BotResponseHandler::sendPlayerGrid(bot.getSocket(), username, channel, game->getPlayerGridObject());
}

void	BotCommands::fireShot( const std::string& username, const std::string& channel, const std::string& msg, Bot& bot )
{
	const std::map<std::string, Game*>& games { bot.getGames() };

	auto it = games.find(username);
	if (it == games.end())
	{
		BotResponseHandler::sendNoGameFeedback(bot.getSocket(), username, channel);
		return;
	}

	std::size_t space_idx = msg.find_first_of(" ");
	if (space_idx == std::string::npos)
	{
		BotResponseHandler::sendNoTargetFeedback(bot.getSocket(), username, channel);
		return;
	}

	std::size_t end_idx = msg.find_first_of(" \r\n", space_idx + 1);

	Game* game { it->second };
	std::string target { msg.substr(space_idx + 1, end_idx - space_idx - 1) };

	if (!game->validInput(target))
	{
		BotResponseHandler::sendInvalidTargetFeedback(bot.getSocket(), username, channel, target);
		return;
	}

	ShotResult sr { game->processShot(target) };

	BotResponseHandler::sendPlayerGrid(bot.getSocket(), username, channel, game->getPlayerGridObject());

	switch (sr)
	{
	case ShotResult::MISS:
		BotResponseHandler::sendMissFeedback(bot.getSocket(), username, channel, target);
		break;
	case ShotResult::HIT:
		BotResponseHandler::sendHitFeedback(bot.getSocket(), username, channel, target);
		break;
	case ShotResult::SUNK:
		BotResponseHandler::sendSunkFeedback(bot.getSocket(), username, channel, game->getSunkName());
		break;
	case ShotResult::WON:
		BotResponseHandler::sendWonFeedback(bot.getSocket(), username, channel);
		bot.removeGame(username);
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
		BotResponseHandler::sendNoGameFeedback(bot.getSocket(), username, channel);
		return;
	}

	Game* game { it->second };

	BotResponseHandler::sendPlayerGrid(bot.getSocket(), username, channel, game->getPlayerGridObject());
}

void	BotCommands::showSolution( const std::string& username, const std::string& channel, const Bot& bot )
{
	const std::map<std::string, Game*>& games { bot.getGames() };

	auto it = games.find(username);
	if (it == games.end())
	{
		BotResponseHandler::sendNoGameFeedback(bot.getSocket(), username, channel);
		return;
	}

	Game* game { it->second };

	BotResponseHandler::sendSolution(bot.getSocket(), username, channel, game->getGridObject());
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

	BotResponseHandler::sendPlayerGrid(bot.getSocket(), username, channel, game->getPlayerGridObject());
}

void BotCommands::help( const std::string& username, const std::string& channel, const Bot& bot )
{
	for ( const auto& cmd : k_help_content )
		BotResponseHandler::sendHelp(bot.getSocket(), username, channel, cmd);
}





/* ===================== Multiplayer Bot Commands ===================== */

void	BotCommands::challenge( const std::string& challenger, const std::string& channel, const std::string& msg, Bot& bot )
{
	std::size_t space_idx = msg.find_first_of(" ");
	if ( space_idx == std::string::npos )
	{
		BotResponseHandler::sendNoChallengedFeedback(bot, challenger, channel);
		return;
	}

	std::size_t end_idx = msg.find_first_of(" \r\n", space_idx + 1);

	std::string challenged { msg.substr(space_idx + 1, end_idx - space_idx - 1) };

	if ( challenger == challenged )
	{
		BotResponseHandler::sendCannotChallengeSelfFeedback(bot, challenger, channel);
		return;
	}

	if ( gameAlreadyExists(challenger, challenged, bot) )
	{
		BotResponseHandler::sendMPGameAlreadyRunningFeedback(bot, challenger, challenged, channel );
		return;
	}

	if ( challengeExists(challenger, challenged, bot) )
	{
		BotResponseHandler::sendAlreadyChallengedFeedback(bot, challenger, channel, challenged);
		return;
	}

	BotResponseHandler::sendChallenge(bot, challenger, challenged, channel);

	bot.addChallenge(challenger, challenged);
}

void	BotCommands::acceptChallenge( const std::string& challenged, const std::string& channel, const std::string& msg, Bot& bot )
{
	std::size_t space_idx = msg.find_first_of(" ");
	if ( space_idx == std::string::npos )
	{
		BotResponseHandler::sendNoChallengerFeedback(bot, challenged, channel);
		return;
	}

	std::size_t end_idx = msg.find_first_of(" \r\n", space_idx + 1);
	std::string challenger { msg.substr(space_idx + 1, end_idx - space_idx - 1) };

	if ( challenger == challenged )
	{
		BotResponseHandler::sendCannotAcceptSelfFeedback(bot, challenged, channel);
		return;
	}

	if ( gameAlreadyExists(challenger, challenged, bot) )
	{
		BotResponseHandler::sendMPGameAlreadyRunningFeedback(bot, challenged, challenger, channel );
		return;
	}

	if ( !challengeExists(challenger, challenged, bot) )
	{
		BotResponseHandler::sendNoChallengeToAcceptFeedback(bot, challenged, channel, challenger);
		return;
	}

	BotResponseHandler::sendAccept(bot, challenger, challenged, channel);

	bot.removeChallenge(challenger, challenged); // needs to be called when the game has been won instead; or an end game command is called

	startMPGame(challenger, challenged, channel, bot);
}





/* ===================== Helper ===================== */

void BotCommands::startMPGame( const std::string& challenger, const std::string& challenged, const std::string& channel, Bot& bot )
{
	MPGame*	mp_game;

	try
	{
		mp_game = new MPGame {challenged, challenger};
		bot.addMPGame({challenger, challenged}, mp_game);
	}
	catch ( const std::exception& e )
	{
		if ( !channel.empty() )
			BotResponseHandler::sendResponse(bot.getSocket(), "", channel, e.what());
		if ( !bot.memberInChannel(challenger) || channel.empty() )
			BotResponseHandler::sendResponse(bot.getSocket(), challenger, "", e.what());
		if ( !bot.memberInChannel(challenged) || channel.empty() )
			BotResponseHandler::sendResponse(bot.getSocket(), challenged, "", e.what());
		return;
	}

	if ( !channel.empty() )
		BotResponseHandler::sendPlayerGrid(bot.getSocket(), "", channel, mp_game->getPlayerOneShotsGridObject());
	if ( !bot.memberInChannel(challenged) || channel.empty() )
		BotResponseHandler::sendPlayerGrid(bot.getSocket(), challenged, "", mp_game->getPlayerOneShotsGridObject());
	if ( !!bot.memberInChannel(challenger) || channel.empty() )
		BotResponseHandler::sendPlayerGrid(bot.getSocket(), challenger, "", mp_game->getPlayerTwoShotsGridObject());
}





/* ===================== Utility ===================== */

BotCommands::BotCommandType BotCommands::getCmdType( const std::string& command )
{
	auto it { k_commands.find(command) };

	if (it != k_commands.end())
		return it->second;
	if (command[0] == '!')
		return CMD_UNKNOWN;
	return CMD_NOTACMD;
}

bool	BotCommands::challengeExists( const std::string& challenger, const std::string& challenged, const Bot& bot )
{
	const auto challenges { bot.getChallenges() };

	for ( auto it {challenges.begin()}; it != challenges.end(); ++it )
		if ( (it->first == challenger && it-> second == challenged) || (it->first == challenged && it-> second == challenger) )
			return true;

	return false;
}

bool	BotCommands::gameAlreadyExists( const std::string& challenger, const std::string& challenged, const Bot& bot )
{
	const auto& mp_games { bot.getMPGames() };

	if ( mp_games.find({challenger, challenged}) != mp_games.end() )
		return true;
	if ( mp_games.find({challenged, challenger}) != mp_games.end() )
		return true;
	return false;
}
