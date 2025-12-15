/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BotCommands.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 17:08:37 by dlippelt          #+#    #+#             */
/*   Updated: 2025/12/15 17:35:47 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BotCommands.hpp"
#include <fstream>

/* ==================== Constructors & Destructors ==================== */

BotCommands::~BotCommands() = default;

BotCommands::BotCommands( Bot& bot, const std::string& username, const std::string& channel, const std::string& message )
	: m_bot { bot }
	, m_username { username }
	, m_channel { channel }
	, m_message { message }
{
}

/* ===================== Bot Commands ===================== */

void	BotCommands::executeCommand()
{
	std::string cmd { m_message.substr(0, m_message.find_first_of(" ")) };

	switch ( getCmdType(cmd) )
	{
	case CMD_START:
		start();
		break;
	case CMD_FIRE:
		fire();
		break;
	case CMD_BOARD:
		board();
		break;
	case CMD_SOLUTION:
		solution();
		break;
	case CMD_NEWGAME:
		newGame();
		break;
	case CMD_HELP:
		help();
		break;
	case CMD_CHALLENGE:
		challenge(m_username);
		break;
	case CMD_ACCEPT:
		acceptChallenge(m_username);
		break;
	case CMD_SHOOT:
		shoot();
		break;
	case CMD_SURRENDER:
		surrender();
		break;
	case CMD_FLEET:
		fleet();
		break;
	case CMD_SHOTS:
		shots();
		break;
	case CMD_FILES:
		files();
		break;
	case CMD_FILE:
		file();
		break;
	case CMD_UNKNOWN:
		BotResponseHandler::sendUnknownCmdFeedback(m_bot.getSocket(), m_username, m_channel, cmd);
		break;
	default:
		break;
	}
}

void	BotCommands::start()
{
	Game*	game;

	try
	{
		const std::map<std::string, Game*>& games { m_bot.getGames() };

		auto it = games.find(m_username);

		if ( it == games.end() )
		{
			game = new Game {};
			m_bot.addGame(m_username, game);
		}
		else
		{
			BotResponseHandler::sendGameAlreadyRunningFeedback(m_bot.getSocket(), m_username, m_channel);
			return;
		}
	}
	catch ( const std::exception& e )
	{
		BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, m_channel, e.what());
		return;
	}

	BotResponseHandler::sendStart(m_bot.getSocket(), m_username, m_channel, game->getPlayerGridObject());
}

void	BotCommands::fire()
{
	const std::map<std::string, Game*>& games { m_bot.getGames() };

	auto it = games.find(m_username);
	if (it == games.end())
	{
		BotResponseHandler::sendNoGameFeedback(m_bot.getSocket(), m_username, m_channel);
		return;
	}

	std::size_t space_idx = m_message.find_first_of(" ");
	if (space_idx == std::string::npos)
	{
		BotResponseHandler::sendNoTargetFeedback(m_bot.getSocket(), m_username, m_channel);
		return;
	}

	std::size_t end_idx = m_message.find_first_of(" \r\n", space_idx + 1);

	Game* game { it->second };
	std::string target { m_message.substr(space_idx + 1, end_idx - space_idx - 1) };

	if (!game->validInput(target))
	{
		BotResponseHandler::sendInvalidTargetFeedback(m_bot.getSocket(), m_username, m_channel, target);
		return;
	}

	ShotResult sr { game->processShot(target) };

	BotResponseHandler::sendFire(m_bot.getSocket(), m_username, m_channel, game->getPlayerGridObject());

	switch (sr)
	{
	case ShotResult::MISS:
		BotResponseHandler::sendMissFeedback(m_bot.getSocket(), m_username, target);
		break;
	case ShotResult::HIT:
		BotResponseHandler::sendHitFeedback(m_bot.getSocket(), m_username, target);
		break;
	case ShotResult::SUNK:
		BotResponseHandler::sendSunkFeedback(m_bot.getSocket(), m_username, game->getSunkName());
		break;
	case ShotResult::WON:
		BotResponseHandler::sendWonFeedback(m_bot.getSocket(), m_username);
		m_bot.removeGame(m_username);
		break;
	default:
		break;
	}
}

void	BotCommands::board()
{
	const std::map<std::string, Game*>& games { m_bot.getGames() };

	auto it = games.find(m_username);
	if (it == games.end())
	{
		BotResponseHandler::sendNoGameFeedback(m_bot.getSocket(), m_username, m_channel);
		return;
	}

	Game* game { it->second };

	BotResponseHandler::sendBoard(m_bot.getSocket(), m_username, m_channel, game->getPlayerGridObject());
}

void	BotCommands::solution()
{
	const std::map<std::string, Game*>& games { m_bot.getGames() };

	auto it = games.find(m_username);
	if (it == games.end())
	{
		BotResponseHandler::sendNoGameFeedback(m_bot.getSocket(), m_username, m_channel);
		return;
	}

	Game* game { it->second };

	BotResponseHandler::sendSolution(m_bot.getSocket(), m_username, m_channel, game->getGridObject());
}

void	BotCommands::newGame()
{
	Game*	game;

	try
	{
		const std::map<std::string, Game*>& games { m_bot.getGames() };

		auto it = games.find(m_username);

		if ( it == games.end() )
		{
			game = new Game {};
			m_bot.addGame(m_username, game);
		}
		else
		{
			m_bot.removeGame(m_username);
			game = new Game {};
			m_bot.addGame(m_username, game);
		}
	}
	catch ( const std::exception& e )
	{
		BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, m_channel, e.what());
		return;
	}

	BotResponseHandler::sendNewGame(m_bot.getSocket(), m_username, m_channel, game->getPlayerGridObject());
}

void BotCommands::help()
{
	if ( !m_channel.empty() )
		BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, m_channel, "An overview of available commands was sent to your DMs, " COLOR RED + m_username + RESET ".");

	for ( const auto& cmd : k_help_content )
		BotResponseHandler::sendHelp(m_bot.getSocket(), m_username, cmd);
}





/* ===================== Multiplayer Bot Commands ===================== */

void	BotCommands::challenge( const std::string& challenger )
{
	std::size_t space_idx = m_message.find_first_of(" ");
	if ( space_idx == std::string::npos )
	{
		BotResponseHandler::sendNoChallengedFeedback(m_bot, challenger, m_channel);
		return;
	}

	std::size_t end_idx = m_message.find_first_of(" \r\n", space_idx + 1);
	std::string challenged { m_message.substr(space_idx + 1, end_idx - space_idx - 1) };

	if ( challenger == challenged )
	{
		BotResponseHandler::sendCannotChallengeSelfFeedback(m_bot, challenger, m_channel);
		return;
	}

	if ( gameAlreadyExists(challenger, challenged) )
	{
		BotResponseHandler::sendMPGameAlreadyRunningFeedback(m_bot, challenger, challenged, m_channel );
		return;
	}

	if ( challengeExists(challenger, challenged) )
	{
		BotResponseHandler::sendAlreadyChallengedFeedback(m_bot, challenger, m_channel, challenged);
		return;
	}

	BotResponseHandler::sendChallenge(m_bot, challenger, challenged, m_channel);

	m_bot.addChallenge(challenger, challenged);
}

void	BotCommands::acceptChallenge( const std::string& challenged )
{
	std::size_t space_idx = m_message.find_first_of(" ");
	if ( space_idx == std::string::npos )
	{
		BotResponseHandler::sendNoChallengerFeedback(m_bot, challenged, m_channel);
		return;
	}

	std::size_t end_idx = m_message.find_first_of(" \r\n", space_idx + 1);
	std::string challenger { m_message.substr(space_idx + 1, end_idx - space_idx - 1) };

	if ( challenger == challenged )
	{
		BotResponseHandler::sendCannotAcceptSelfFeedback(m_bot, challenged, m_channel);
		return;
	}

	if ( gameAlreadyExists(challenger, challenged) )
	{
		BotResponseHandler::sendMPGameAlreadyRunningFeedback(m_bot, challenged, challenger, m_channel );
		return;
	}

	if ( !challengeExists(challenger, challenged) )
	{
		BotResponseHandler::sendNoChallengeToAcceptFeedback(m_bot, challenged, m_channel, challenger);
		return;
	}

	BotResponseHandler::sendAccept(m_bot, challenger, challenged, m_channel);

	startMPGame(challenger, challenged);
}

void	BotCommands::shoot()
{
	const auto&			mp_games { m_bot.getMPGames() };
	const std::string	opponent { getOpponentName() };
	const std::string	target { getMPTarget() };

	if ( m_username == opponent )
	{
		BotResponseHandler::sendCannotShootSelf(m_bot, m_username, m_channel);
		return;
	}

	auto it	{ m_bot.getMPGame(m_username, opponent) };
	if ( it == mp_games.end() )
	{
		BotResponseHandler::sendNoMPGameFeedback(m_bot, m_username, opponent, m_channel);
		return;
	}

	MPGame* mp_game { it->second };

	if ( m_username != mp_game->getCurrentPlayer() )
	{
		BotResponseHandler::sendNotYourTurnFeedback(m_bot, m_username, opponent, m_channel);
		return;
	}

	if ( !mp_game->validInput(target, m_username) )
	{
		BotResponseHandler::sendInvalidTargetFeedback(m_bot.getSocket(), m_username, m_channel, target);
		return;
	}

	ShotResult sr { mp_game->processShot(target, m_username) };

	BotResponseHandler::sendShot(m_bot, m_username, m_channel, opponent, *mp_game->getPlayerShotsGridObject(m_username), *mp_game->getPlayerGridObject(opponent));

	switch (sr)
	{
	case ShotResult::MISS:
		BotResponseHandler::sendMissFeedback(m_bot.getSocket(), m_username, target, opponent);
		break;
	case ShotResult::HIT:
		BotResponseHandler::sendHitFeedback(m_bot.getSocket(), m_username, target, opponent);
		break;
	case ShotResult::SUNK:
		BotResponseHandler::sendSunkFeedback(m_bot.getSocket(), m_username, mp_game->getSunkName(), opponent);
		break;
	case ShotResult::WON:
		BotResponseHandler::sendWonFeedback(m_bot.getSocket(), m_username, opponent);
		m_bot.removeMPGame(m_username, opponent);
		m_bot.removeChallenge(m_username, opponent);
		return;
	default:
		break;
	}

	BotResponseHandler::sendTurnInfo(m_bot, m_username, opponent, mp_game);
}

void	BotCommands::surrender()
{
	const auto&			mp_games { m_bot.getMPGames() };
	const std::string	opponent { getOpponentName() };

	if ( m_username == opponent )
	{
		BotResponseHandler::sendCannotSurrenderToSelfFeedback(m_bot, opponent, m_channel);
		return;
	}

	auto it	{ m_bot.getMPGame(m_username, opponent) };
	if ( it == mp_games.end() )
	{
		BotResponseHandler::sendNoMPGameFeedback(m_bot, m_username, opponent, m_channel);
		return;
	}

	BotResponseHandler::sendSurrender(m_bot, m_username, opponent, m_channel);
	m_bot.removeMPGame(m_username, opponent);
	m_bot.removeChallenge(m_username, opponent);
}

void	BotCommands::fleet()
{
	const auto&			mp_games { m_bot.getMPGames() };
	const std::string	opponent { getOpponentName() };

	if ( m_username == opponent )
	{
		BotResponseHandler::sendCannotShowFleetOrShotsAgainstSelf(m_bot, opponent, m_channel);
		return;
	}

	auto it	{ m_bot.getMPGame(m_username, opponent) };
	if ( it == mp_games.end() )
	{
		BotResponseHandler::sendNoMPGameFeedback(m_bot, m_username, opponent, m_channel);
		return;
	}

	MPGame* mp_game { it->second };

	BotResponseHandler::sendFleet(m_bot, m_username, m_channel, opponent, *mp_game->getPlayerGridObject(m_username));
}

void	BotCommands::shots()
{
	const auto&			mp_games { m_bot.getMPGames() };
	const std::string	opponent { getOpponentName() };

	if ( m_username == opponent )
	{
		BotResponseHandler::sendCannotShowFleetOrShotsAgainstSelf(m_bot, opponent, m_channel);
		return;
	}

	auto it	{ m_bot.getMPGame(m_username, opponent) };
	if ( it == mp_games.end() )
	{
		BotResponseHandler::sendNoMPGameFeedback(m_bot, m_username, opponent, m_channel);
		return;
	}

	MPGame* mp_game { it->second };

	BotResponseHandler::sendShots(m_bot, m_username, m_channel, opponent, *mp_game->getPlayerShotsGridObject(m_username));
}





/* ===================== Helper ===================== */

void BotCommands::startMPGame( const std::string& challenger, const std::string& challenged )
{
	MPGame*	mp_game;

	try
	{
		mp_game = new MPGame {challenger, challenged};
		m_bot.addMPGame({challenger, challenged}, mp_game);
	}
	catch ( const std::exception& e )
	{
		if ( !m_channel.empty() )
			BotResponseHandler::sendResponse(m_bot.getSocket(), "", m_channel, e.what());
		if ( !m_bot.memberInChannel(challenger) || m_channel.empty() )
			BotResponseHandler::sendResponse(m_bot.getSocket(), challenger, "", e.what());
		if ( !m_bot.memberInChannel(challenged) || m_channel.empty() )
			BotResponseHandler::sendResponse(m_bot.getSocket(), challenged, "", e.what());
		return;
	}

	BotResponseHandler::sendPlayerGrid(m_bot.getSocket(), challenger, *mp_game->getPlayerGridObject(challenger), BotResponseHandler::GridType::REFERENCE, challenged);
	BotResponseHandler::sendPlayerGrid(m_bot.getSocket(), challenger, *mp_game->getPlayerShotsGridObject(challenger), BotResponseHandler::GridType::TRACKING, challenged);
	BotResponseHandler::sendPlayerGrid(m_bot.getSocket(), challenged, *mp_game->getPlayerGridObject(challenged), BotResponseHandler::GridType::REFERENCE, challenger);
	BotResponseHandler::sendPlayerGrid(m_bot.getSocket(), challenged, *mp_game->getPlayerShotsGridObject(challenged), BotResponseHandler::GridType::TRACKING, challenger);

	BotResponseHandler::sendTurnInfo(m_bot, challenger, challenged, mp_game);

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

bool	BotCommands::challengeExists( const std::string& challenger, const std::string& challenged )
{
	const auto challenges { m_bot.getChallenges() };

	for ( auto it {challenges.begin()}; it != challenges.end(); ++it )
		if ( (it->first == challenger && it-> second == challenged) || (it->first == challenged && it-> second == challenger) )
			return true;

	return false;
}

bool	BotCommands::gameAlreadyExists( const std::string& challenger, const std::string& challenged )
{
	const auto& mp_games { m_bot.getMPGames() };

	if ( mp_games.find({challenger, challenged}) != mp_games.end() )
		return true;
	if ( mp_games.find({challenged, challenger}) != mp_games.end() )
		return true;
	return false;
}

std::string	BotCommands::getOpponentName()
{
	std::size_t	start_idx { m_message.find_first_of(" \r\n") };
	if ( start_idx == std::string::npos )
		return ("");

	start_idx++;

	std::size_t	end_idx { m_message.find_first_of(" \r\n", start_idx) };

	return ( m_message.substr(start_idx, end_idx - start_idx) );
}

std::string	BotCommands::getMPTarget()
{
	std::size_t	start_idx { m_message.find_first_of(" \r\n") + 1 };
	start_idx = m_message.find_first_of(" \r\n", start_idx);
	if ( start_idx == std::string::npos )
		return ("");

	start_idx++;

	std::size_t	end_idx { m_message.find_first_of(" \r\n", start_idx) };

	return ( m_message.substr(start_idx, end_idx - start_idx) );
}

/* ===================== File Service Bot Commands ===================== */

void BotCommands::files()
{
	if ( !m_channel.empty() )
		BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, m_channel,
			"Available files list sent to your DMs, " COLOR RED + m_username + RESET ".");

	BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, "", "");
	BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, "",
		COLOR LIGHT_CYAN "=== Available Files ===" RESET);
	BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, "", "");
	BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, "",
		COLOR YELLOW "welcome.txt" RESET " - Welcome message and file list");
	BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, "",
		COLOR YELLOW "rules.txt" RESET " - Complete game rules and commands");
	BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, "",
		COLOR YELLOW "tips.txt" RESET " - Strategy tips and tactics");
	BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, "",
		COLOR YELLOW "commands.txt" RESET " - Full command reference");
	BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, "",
		COLOR YELLOW "history.txt" RESET " - History of Battleships game");
	BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, "", "");
	BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, "",
		"Use " COLOR YELLOW "!file <filename>" RESET " to download.");
	BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, "",
		"Example: " COLOR YELLOW "!file rules.txt" RESET);
	BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, "", "");
}

void BotCommands::file()
{
	std::size_t space_idx = m_message.find_first_of(" ");
	if ( space_idx == std::string::npos )
	{
		BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, m_channel, "");
		BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, m_channel,
			"Please specify a filename (e.g. " COLOR YELLOW "!file rules.txt" RESET ").");
		BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, m_channel,
			"Type " COLOR YELLOW "!files" RESET " to see available files.");
		BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, m_channel, "");
		return;
	}

	std::size_t end_idx = m_message.find_first_of(" \r\n", space_idx + 1);
	std::string filename { m_message.substr(space_idx + 1, end_idx - space_idx - 1) };

	if ( filename.find("..") != std::string::npos || filename.find("/") != std::string::npos )
	{
		BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, m_channel, "");
		BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, m_channel,
			COLOR RED "Invalid filename." RESET);
		BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, m_channel, "");
		return;
	}

	std::string filepath { "Bot/bot_files/" + filename };
	std::ifstream file(filepath);

	if ( !file.is_open() )
	{
		BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, m_channel, "");
		BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, m_channel,
			"File " COLOR RED "'" + filename + "'" RESET " not found.");
		BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, m_channel,
			"Type " COLOR YELLOW "!files" RESET " to see available files.");
		BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, m_channel, "");
		return;
	}

	if ( !m_channel.empty() )
		BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, m_channel,
			"File " COLOR GREEN "'" + filename + "'" RESET " sent to your DMs, " COLOR RED + m_username + RESET ".");

	BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, "", "");
	BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, "",
		COLOR LIGHT_CYAN "=== " + filename + " ===" RESET);
	BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, "", "");

	std::string line;
	while ( std::getline(file, line) )
		BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, "", line);

	BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, "", "");
	BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, "",
		COLOR LIGHT_CYAN "=== End of " + filename + " ===" RESET);
	BotResponseHandler::sendResponse(m_bot.getSocket(), m_username, "", "");

	file.close();
}
