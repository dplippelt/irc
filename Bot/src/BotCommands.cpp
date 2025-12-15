/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   BotCommands.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: dlippelt <dlippelt@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/25 17:08:37 by dlippelt      #+#    #+#                 */
/*   Updated: 2025/12/15 15:31:23 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "BotCommands.hpp"
#include <fstream>

/* ===================== Bot Commands ===================== */

void	BotCommands::executeCommand( const std::string& username, const std::string& channel, const std::string& message, Bot& bot )
{
	std::string cmd { message.substr(0, message.find_first_of(" ")) };

	switch ( getCmdType(cmd) )
	{
	case CMD_START:
		start(username, channel, bot);
		break;
	case CMD_FIRE:
		fire(username, channel, message, bot);
		break;
	case CMD_BOARD:
		board(username, channel, bot);
		break;
	case CMD_SOLUTION:
		solution(username, channel, bot);
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
	case CMD_SHOOT:
		shoot(username, channel, message, bot);
		break;
	case CMD_SURRENDER:
		surrender(username, channel, message, bot);
		break;
	case CMD_FLEET:
		fleet(username, channel, message, bot);
		break;
	case CMD_SHOTS:
		shots(username, channel, message, bot);
		break;
	case CMD_FILES:
		files(username, channel, bot);
		break;
	case CMD_FILE:
		file(username, channel, message, bot);
		break;
	case CMD_UNKNOWN:
		BotResponseHandler::sendUnknownCmdFeedback(bot.getSocket(), username, channel, cmd);
		break;
	default:
		break;
	}
}

void	BotCommands::start( const std::string& username, const std::string& channel, Bot& bot )
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

	BotResponseHandler::sendStart(bot.getSocket(), username, channel, game->getPlayerGridObject());
}

void	BotCommands::fire( const std::string& username, const std::string& channel, const std::string& msg, Bot& bot )
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

	BotResponseHandler::sendFire(bot.getSocket(), username, channel, game->getPlayerGridObject());

	switch (sr)
	{
	case ShotResult::MISS:
		BotResponseHandler::sendMissFeedback(bot.getSocket(), username, target);
		break;
	case ShotResult::HIT:
		BotResponseHandler::sendHitFeedback(bot.getSocket(), username, target);
		break;
	case ShotResult::SUNK:
		BotResponseHandler::sendSunkFeedback(bot.getSocket(), username, game->getSunkName());
		break;
	case ShotResult::WON:
		BotResponseHandler::sendWonFeedback(bot.getSocket(), username);
		bot.removeGame(username);
		break;
	default:
		break;
	}
}

void	BotCommands::board( const std::string& username, const std::string& channel, const Bot& bot )
{
	const std::map<std::string, Game*>& games { bot.getGames() };

	auto it = games.find(username);
	if (it == games.end())
	{
		BotResponseHandler::sendNoGameFeedback(bot.getSocket(), username, channel);
		return;
	}

	Game* game { it->second };

	BotResponseHandler::sendBoard(bot.getSocket(), username, channel, game->getPlayerGridObject());
}

void	BotCommands::solution( const std::string& username, const std::string& channel, const Bot& bot )
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

	BotResponseHandler::sendNewGame(bot.getSocket(), username, channel, game->getPlayerGridObject());
}

void BotCommands::help( const std::string& username, const std::string& channel, const Bot& bot )
{
	if ( !channel.empty() )
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "An overview of available commands was sent to your DMs, " COLOR RED + username + RESET ".");

	for ( const auto& cmd : k_help_content )
		BotResponseHandler::sendHelp(bot.getSocket(), username, cmd);
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

	startMPGame(challenger, challenged, channel, bot);
}

void	BotCommands::shoot( const std::string& username, const std::string& channel, const std::string& msg, Bot& bot )
{
	const auto&			mp_games { bot.getMPGames() };
	const std::string	opponent { getOpponentName(msg) };
	const std::string	target { getMPTarget(msg) };

	if ( username == opponent )
	{
		BotResponseHandler::sendCannotShootSelf(bot, username, channel);
		return;
	}

	auto it	{ bot.getMPGame(username, opponent) };
	if ( it == mp_games.end() )
	{
		BotResponseHandler::sendNoMPGameFeedback(bot, username, opponent, channel);
		return;
	}

	MPGame* mp_game { it->second };

	if ( username != mp_game->getCurrentPlayer() )
	{
		BotResponseHandler::sendNotYourTurnFeedback(bot, username, opponent, channel);
		return;
	}

	if ( !mp_game->validInput(target, username) )
	{
		BotResponseHandler::sendInvalidTargetFeedback(bot.getSocket(), username, channel, target);
		return;
	}

	ShotResult sr { mp_game->processShot(target, username) };

	BotResponseHandler::sendShot(bot, username, channel, opponent, *mp_game->getPlayerShotsGridObject(username), *mp_game->getPlayerGridObject(opponent));

	switch (sr)
	{
	case ShotResult::MISS:
		BotResponseHandler::sendMissFeedback(bot.getSocket(), username, target, opponent);
		break;
	case ShotResult::HIT:
		BotResponseHandler::sendHitFeedback(bot.getSocket(), username, target, opponent);
		break;
	case ShotResult::SUNK:
		BotResponseHandler::sendSunkFeedback(bot.getSocket(), username, mp_game->getSunkName(), opponent);
		break;
	case ShotResult::WON:
		BotResponseHandler::sendWonFeedback(bot.getSocket(), username, opponent);
		bot.removeMPGame(username, opponent);
		bot.removeChallenge(username, opponent);
		return;
	default:
		break;
	}

	BotResponseHandler::sendTurnInfo(bot, username, opponent, mp_game);
}

void	BotCommands::surrender( const std::string& username, const std::string& channel, const std::string& msg, Bot& bot )
{
	const auto&			mp_games { bot.getMPGames() };
	const std::string	opponent { getOpponentName(msg) };

	if ( username == opponent )
	{
		BotResponseHandler::sendCannotSurrenderToSelfFeedback(bot, opponent, channel);
		return;
	}

	auto it	{ bot.getMPGame(username, opponent) };
	if ( it == mp_games.end() )
	{
		BotResponseHandler::sendNoMPGameFeedback(bot, username, opponent, channel);
		return;
	}

	BotResponseHandler::sendSurrender(bot, username, opponent, channel);
	bot.removeMPGame(username, opponent);
	bot.removeChallenge(username, opponent);
}

void	BotCommands::fleet( const std::string& username, const std::string& channel, const std::string& msg, Bot& bot )
{
	const auto&			mp_games { bot.getMPGames() };
	const std::string	opponent { getOpponentName(msg) };

	if ( username == opponent )
	{
		BotResponseHandler::sendCannotShowFleetOrShotsAgainstSelf(bot, opponent, channel);
		return;
	}

	auto it	{ bot.getMPGame(username, opponent) };
	if ( it == mp_games.end() )
	{
		BotResponseHandler::sendNoMPGameFeedback(bot, username, opponent, channel);
		return;
	}

	MPGame* mp_game { it->second };

	BotResponseHandler::sendFleet(bot, username, channel, opponent, *mp_game->getPlayerGridObject(username));
}

void	BotCommands::shots( const std::string& username, const std::string& channel, const std::string& msg, Bot& bot )
{
	const auto&			mp_games { bot.getMPGames() };
	const std::string	opponent { getOpponentName(msg) };

	if ( username == opponent )
	{
		BotResponseHandler::sendCannotShowFleetOrShotsAgainstSelf(bot, opponent, channel);
		return;
	}

	auto it	{ bot.getMPGame(username, opponent) };
	if ( it == mp_games.end() )
	{
		BotResponseHandler::sendNoMPGameFeedback(bot, username, opponent, channel);
		return;
	}

	MPGame* mp_game { it->second };

	BotResponseHandler::sendShots(bot, username, channel, opponent, *mp_game->getPlayerShotsGridObject(username));
}





/* ===================== Helper ===================== */

void BotCommands::startMPGame( const std::string& challenger, const std::string& challenged, const std::string& channel, Bot& bot )
{
	MPGame*	mp_game;

	try
	{
		mp_game = new MPGame {challenger, challenged};
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

	BotResponseHandler::sendPlayerGrid(bot.getSocket(), challenger, *mp_game->getPlayerGridObject(challenger), BotResponseHandler::GridType::REFERENCE, challenged);
	BotResponseHandler::sendPlayerGrid(bot.getSocket(), challenger, *mp_game->getPlayerShotsGridObject(challenger), BotResponseHandler::GridType::TRACKING, challenged);
	BotResponseHandler::sendPlayerGrid(bot.getSocket(), challenged, *mp_game->getPlayerGridObject(challenged), BotResponseHandler::GridType::REFERENCE, challenger);
	BotResponseHandler::sendPlayerGrid(bot.getSocket(), challenged, *mp_game->getPlayerShotsGridObject(challenged), BotResponseHandler::GridType::TRACKING, challenger);

	BotResponseHandler::sendTurnInfo(bot, challenger, challenged, mp_game);

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

std::string	BotCommands::getOpponentName( const std::string& msg )
{
	std::size_t	start_idx { msg.find_first_of(" \r\n") };
	if ( start_idx == std::string::npos )
		return ("");

	start_idx++;

	std::size_t	end_idx { msg.find_first_of(" \r\n", start_idx) };

	return ( msg.substr(start_idx, end_idx - start_idx) );
}

std::string	BotCommands::getMPTarget( const std::string& msg )
{
	std::size_t	start_idx { msg.find_first_of(" \r\n") + 1 };
	start_idx = msg.find_first_of(" \r\n", start_idx);
	if ( start_idx == std::string::npos )
		return ("");

	start_idx++;

	std::size_t	end_idx { msg.find_first_of(" \r\n", start_idx) };

	return ( msg.substr(start_idx, end_idx - start_idx) );
}

/* ===================== File Service Bot Commands ===================== */

void BotCommands::files( const std::string& username, const std::string& channel, const Bot& bot )
{
	if ( !channel.empty() )
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel,
			"Available files list sent to your DMs, " COLOR RED + username + RESET ".");

	BotResponseHandler::sendResponse(bot.getSocket(), username, "", "");
	BotResponseHandler::sendResponse(bot.getSocket(), username, "",
		COLOR LIGHT_CYAN "=== Available Files ===" RESET);
	BotResponseHandler::sendResponse(bot.getSocket(), username, "", "");
	BotResponseHandler::sendResponse(bot.getSocket(), username, "",
		COLOR YELLOW "welcome.txt" RESET " - Welcome message and file list");
	BotResponseHandler::sendResponse(bot.getSocket(), username, "",
		COLOR YELLOW "rules.txt" RESET " - Complete game rules and commands");
	BotResponseHandler::sendResponse(bot.getSocket(), username, "",
		COLOR YELLOW "tips.txt" RESET " - Strategy tips and tactics");
	BotResponseHandler::sendResponse(bot.getSocket(), username, "",
		COLOR YELLOW "commands.txt" RESET " - Full command reference");
	BotResponseHandler::sendResponse(bot.getSocket(), username, "",
		COLOR YELLOW "history.txt" RESET " - History of Battleships game");
	BotResponseHandler::sendResponse(bot.getSocket(), username, "", "");
	BotResponseHandler::sendResponse(bot.getSocket(), username, "",
		"Use " COLOR YELLOW "!file <filename>" RESET " to download.");
	BotResponseHandler::sendResponse(bot.getSocket(), username, "",
		"Example: " COLOR YELLOW "!file rules.txt" RESET);
	BotResponseHandler::sendResponse(bot.getSocket(), username, "", "");
}

void BotCommands::file( const std::string& username, const std::string& channel, const std::string& msg, Bot& bot )
{
	std::size_t space_idx = msg.find_first_of(" ");
	if ( space_idx == std::string::npos )
	{
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "");
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel,
			"Please specify a filename (e.g. " COLOR YELLOW "!file rules.txt" RESET ").");
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel,
			"Type " COLOR YELLOW "!files" RESET " to see available files.");
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "");
		return;
	}

	std::size_t end_idx = msg.find_first_of(" \r\n", space_idx + 1);
	std::string filename { msg.substr(space_idx + 1, end_idx - space_idx - 1) };

	if ( filename.find("..") != std::string::npos || filename.find("/") != std::string::npos )
	{
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "");
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel,
			COLOR RED "Invalid filename." RESET);
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "");
		return;
	}

	std::string filepath { "Bot/bot_files/" + filename };
	std::ifstream file(filepath);

	if ( !file.is_open() )
	{
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "");
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel,
			"File " COLOR RED "'" + filename + "'" RESET " not found.");
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel,
			"Type " COLOR YELLOW "!files" RESET " to see available files.");
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel, "");
		return;
	}

	if ( !channel.empty() )
		BotResponseHandler::sendResponse(bot.getSocket(), username, channel,
			"File " COLOR GREEN "'" + filename + "'" RESET " sent to your DMs, " COLOR RED + username + RESET ".");

	BotResponseHandler::sendResponse(bot.getSocket(), username, "", "");
	BotResponseHandler::sendResponse(bot.getSocket(), username, "",
		COLOR LIGHT_CYAN "=== " + filename + " ===" RESET);
	BotResponseHandler::sendResponse(bot.getSocket(), username, "", "");

	std::string line;
	while ( std::getline(file, line) )
		BotResponseHandler::sendResponse(bot.getSocket(), username, "", line);

	BotResponseHandler::sendResponse(bot.getSocket(), username, "", "");
	BotResponseHandler::sendResponse(bot.getSocket(), username, "",
		COLOR LIGHT_CYAN "=== End of " + filename + " ===" RESET);
	BotResponseHandler::sendResponse(bot.getSocket(), username, "", "");

	file.close();
}
