/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BotCommands.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 17:08:37 by dlippelt          #+#    #+#             */
/*   Updated: 2026/01/15 14:37:38 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BotCommands.hpp"
#include "BotResponseHandler.hpp"
#include <fstream>

/* ==================== Constructors & Destructors ==================== */

BotCommands::~BotCommands() = default;

BotCommands::BotCommands( Bot& bot, const std::string& username, const std::string& channel, const std::string& message )
	: m_bot { bot }
	, m_responseHandler { std::make_unique<BotResponseHandler>(bot) }
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
		m_responseHandler->sendUnknownCmdFeedback(m_username, m_channel, cmd);
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
			m_responseHandler->sendGameAlreadyRunningFeedback(m_username, m_channel);
			return;
		}
	}
	catch ( const std::exception& e )
	{
		m_responseHandler->sendResponse(m_username, m_channel, e.what());
		return;
	}

	m_responseHandler->sendStart(m_username, m_channel, game->getPlayerGridObject());
}

void	BotCommands::fire()
{
	const std::map<std::string, Game*>& games { m_bot.getGames() };

	auto it = games.find(m_username);
	if (it == games.end())
	{
		m_responseHandler->sendNoGameFeedback(m_username, m_channel);
		return;
	}

	std::size_t space_idx = m_message.find_first_of(" ");
	if (space_idx == std::string::npos)
	{
		m_responseHandler->sendNoTargetFeedback(m_username, m_channel);
		return;
	}

	std::size_t end_idx = m_message.find_first_of(" \r\n", space_idx + 1);

	Game* game { it->second };
	std::string target { m_message.substr(space_idx + 1, end_idx - space_idx - 1) };

	if (!game->validInput(target))
	{
		m_responseHandler->sendInvalidTargetFeedback(m_username, m_channel, target);
		return;
	}

	ShotResult sr { game->processShot(target) };

	m_responseHandler->sendFire(m_username, m_channel, game->getPlayerGridObject());

	switch (sr)
	{
	case ShotResult::MISS:
		m_responseHandler->sendMissFeedback(m_username, target);
		break;
	case ShotResult::HIT:
		m_responseHandler->sendHitFeedback(m_username, target);
		break;
	case ShotResult::SUNK:
		m_responseHandler->sendSunkFeedback(m_username, game->getSunkName());
		break;
	case ShotResult::WON:
		m_responseHandler->sendWonFeedback(m_username);
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
		m_responseHandler->sendNoGameFeedback(m_username, m_channel);
		return;
	}

	Game* game { it->second };

	m_responseHandler->sendBoard(m_username, m_channel, game->getPlayerGridObject());
}

void	BotCommands::solution()
{
	const std::map<std::string, Game*>& games { m_bot.getGames() };

	auto it = games.find(m_username);
	if (it == games.end())
	{
		m_responseHandler->sendNoGameFeedback(m_username, m_channel);
		return;
	}

	Game* game { it->second };

	m_responseHandler->sendSolution(m_username, m_channel, game->getGridObject());
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
		m_responseHandler->sendResponse(m_username, m_channel, e.what());
		return;
	}

	m_responseHandler->sendNewGame(m_username, m_channel, game->getPlayerGridObject());
}

void BotCommands::help()
{
	if ( !m_channel.empty() )
		m_responseHandler->sendResponse(m_username, m_channel, "An overview of available commands was sent to your DMs, " COLOR RED + m_username + RESET ".");

	m_responseHandler->sendResponse(m_username, "",	COLOR LIGHT_CYAN "=== Available Commands ===" RESET);
	for ( const auto& cmd : k_help_content )
		m_responseHandler->sendHelp(m_username, cmd);
}





/* ===================== Multiplayer Bot Commands ===================== */

void	BotCommands::challenge( const std::string& challenger )
{
	std::size_t space_idx = m_message.find_first_of(" ");
	if ( space_idx == std::string::npos )
	{
		m_responseHandler->sendNoChallengedFeedback(challenger, m_channel);
		return;
	}

	std::size_t end_idx = m_message.find_first_of(" \r\n", space_idx + 1);
	std::string challenged { m_message.substr(space_idx + 1, end_idx - space_idx - 1) };

	if ( challenger == challenged )
	{
		m_responseHandler->sendCannotChallengeSelfFeedback(challenger, m_channel);
		return;
	}

	if ( gameAlreadyExists(challenger, challenged) )
	{
		m_responseHandler->sendMPGameAlreadyRunningFeedback(challenger, challenged, m_channel );
		return;
	}

	if ( challengeExists(challenger, challenged) )
	{
		m_responseHandler->sendAlreadyChallengedFeedback(challenger, m_channel, challenged);
		return;
	}

	m_responseHandler->sendChallenge(challenger, challenged, m_channel);

	m_bot.addChallenge(challenger, challenged);
}

void	BotCommands::acceptChallenge( const std::string& challenged )
{
	std::size_t space_idx = m_message.find_first_of(" ");
	if ( space_idx == std::string::npos )
	{
		m_responseHandler->sendNoChallengerFeedback(challenged, m_channel);
		return;
	}

	std::size_t end_idx = m_message.find_first_of(" \r\n", space_idx + 1);
	std::string challenger { m_message.substr(space_idx + 1, end_idx - space_idx - 1) };

	if ( challenger == challenged )
	{
		m_responseHandler->sendCannotAcceptSelfFeedback(challenged, m_channel);
		return;
	}

	if ( gameAlreadyExists(challenger, challenged) )
	{
		m_responseHandler->sendMPGameAlreadyRunningFeedback(challenged, challenger, m_channel );
		return;
	}

	if ( !challengeExists(challenger, challenged) )
	{
		m_responseHandler->sendNoChallengeToAcceptFeedback(challenged, m_channel, challenger);
		return;
	}

	m_responseHandler->sendAccept(challenger, challenged, m_channel);

	startMPGame(challenger, challenged);
}

void	BotCommands::shoot()
{
	const auto&			mp_games { m_bot.getMPGames() };
	const std::string	opponent { getOpponentName() };
	const std::string	target { getMPTarget() };

	if ( m_username == opponent )
	{
		m_responseHandler->sendCannotShootSelf(m_username, m_channel);
		return;
	}

	auto it	{ m_bot.getMPGame(m_username, opponent) };
	if ( it == mp_games.end() )
	{
		m_responseHandler->sendNoMPGameFeedback(m_username, opponent, m_channel);
		return;
	}

	MPGame* mp_game { it->second };

	if ( m_username != mp_game->getCurrentPlayer() )
	{
		m_responseHandler->sendNotYourTurnFeedback(m_username, opponent, m_channel);
		return;
	}

	if ( !mp_game->validInput(target, m_username) )
	{
		m_responseHandler->sendInvalidTargetFeedback(m_username, m_channel, target);
		return;
	}

	ShotResult sr { mp_game->processShot(target, m_username) };

	m_responseHandler->sendShot(m_username, m_channel, opponent, *mp_game->getPlayerShotsGridObject(m_username), *mp_game->getPlayerGridObject(opponent));

	switch (sr)
	{
	case ShotResult::MISS:
		m_responseHandler->sendMissFeedback(m_username, target, opponent);
		break;
	case ShotResult::HIT:
		m_responseHandler->sendHitFeedback(m_username, target, opponent);
		break;
	case ShotResult::SUNK:
		m_responseHandler->sendSunkFeedback(m_username, mp_game->getSunkName(), opponent);
		break;
	case ShotResult::WON:
		m_responseHandler->sendWonFeedback(m_username, opponent);
		m_bot.removeMPGame(m_username, opponent);
		m_bot.removeChallenge(m_username, opponent);
		return;
	default:
		break;
	}

	m_responseHandler->sendPlayerGrid(opponent, *mp_game->getPlayerShotsGridObject(opponent), m_responseHandler->GridType::TRACKING, m_username);
	m_responseHandler->sendTurnInfo(m_username, opponent, mp_game);
}

void	BotCommands::surrender()
{
	const auto&			mp_games { m_bot.getMPGames() };
	const std::string	opponent { getOpponentName() };

	if ( m_username == opponent )
	{
		m_responseHandler->sendCannotSurrenderToSelfFeedback(opponent, m_channel);
		return;
	}

	auto it	{ m_bot.getMPGame(m_username, opponent) };
	if ( it == mp_games.end() )
	{
		m_responseHandler->sendNoMPGameFeedback(m_username, opponent, m_channel);
		return;
	}

	m_responseHandler->sendSurrender(m_username, opponent, m_channel);
	m_bot.removeMPGame(m_username, opponent);
	m_bot.removeChallenge(m_username, opponent);
}

void	BotCommands::fleet()
{
	const auto&			mp_games { m_bot.getMPGames() };
	const std::string	opponent { getOpponentName() };

	if ( m_username == opponent )
	{
		m_responseHandler->sendCannotShowFleetOrShotsAgainstSelf(opponent, m_channel);
		return;
	}

	auto it	{ m_bot.getMPGame(m_username, opponent) };
	if ( it == mp_games.end() )
	{
		m_responseHandler->sendNoMPGameFeedback(m_username, opponent, m_channel);
		return;
	}

	MPGame* mp_game { it->second };

	m_responseHandler->sendFleet(m_username, m_channel, opponent, *mp_game->getPlayerGridObject(m_username));
}

void	BotCommands::shots()
{
	const auto&			mp_games { m_bot.getMPGames() };
	const std::string	opponent { getOpponentName() };

	if ( m_username == opponent )
	{
		m_responseHandler->sendCannotShowFleetOrShotsAgainstSelf(opponent, m_channel);
		return;
	}

	auto it	{ m_bot.getMPGame(m_username, opponent) };
	if ( it == mp_games.end() )
	{
		m_responseHandler->sendNoMPGameFeedback(m_username, opponent, m_channel);
		return;
	}

	MPGame* mp_game { it->second };

	m_responseHandler->sendShots(m_username, m_channel, opponent, *mp_game->getPlayerShotsGridObject(m_username));
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
			m_responseHandler->sendResponse("", m_channel, e.what());
		if ( !m_bot.memberInChannel(challenger) || m_channel.empty() )
			m_responseHandler->sendResponse(challenger, "", e.what());
		if ( !m_bot.memberInChannel(challenged) || m_channel.empty() )
			m_responseHandler->sendResponse(challenged, "", e.what());
		return;
	}

	m_responseHandler->sendPlayerGrid(challenger, *mp_game->getPlayerGridObject(challenger), m_responseHandler->GridType::REFERENCE, challenged);
	m_responseHandler->sendPlayerGrid(challenger, *mp_game->getPlayerShotsGridObject(challenger), m_responseHandler->GridType::TRACKING, challenged);
	m_responseHandler->sendPlayerGrid(challenged, *mp_game->getPlayerGridObject(challenged), m_responseHandler->GridType::REFERENCE, challenger);
	m_responseHandler->sendPlayerGrid(challenged, *mp_game->getPlayerShotsGridObject(challenged), m_responseHandler->GridType::TRACKING, challenger);

	m_responseHandler->sendTurnInfo(challenger, challenged, mp_game);
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
		m_responseHandler->sendResponse(m_username, m_channel,
			"Available files list sent to your DMs, " COLOR RED + m_username + RESET ".");

	m_responseHandler->sendResponse(m_username, "", "");
	m_responseHandler->sendResponse(m_username, "",
		COLOR LIGHT_CYAN "=== Available Files ===" RESET);
	m_responseHandler->sendResponse(m_username, "", "");
	m_responseHandler->sendResponse(m_username, "",
		COLOR YELLOW "welcome.txt" RESET " - Welcome message and file list");
	m_responseHandler->sendResponse(m_username, "",
		COLOR YELLOW "rules.txt" RESET " - Complete game rules and commands");
	m_responseHandler->sendResponse(m_username, "",
		COLOR YELLOW "tips.txt" RESET " - Strategy tips and tactics");
	m_responseHandler->sendResponse(m_username, "",
		COLOR YELLOW "commands.txt" RESET " - Full command reference");
	m_responseHandler->sendResponse(m_username, "", "");
	m_responseHandler->sendResponse(m_username, "",
		"Use " COLOR YELLOW "!file <filename>" RESET " to download.");
	m_responseHandler->sendResponse(m_username, "",
		"Example: " COLOR YELLOW "!file rules.txt" RESET);
	m_responseHandler->sendResponse(m_username, "", "");
}

void BotCommands::file()
{
	std::size_t space_idx = m_message.find_first_of(" ");
	if ( space_idx == std::string::npos )
	{
		m_responseHandler->sendResponse(m_username, m_channel, "");
		m_responseHandler->sendResponse(m_username, m_channel,
			"Please specify a filename (e.g. " COLOR YELLOW "!file rules.txt" RESET ").");
		m_responseHandler->sendResponse(m_username, m_channel,
			"Type " COLOR YELLOW "!files" RESET " to see available files.");
		m_responseHandler->sendResponse(m_username, m_channel, "");
		return;
	}

	std::size_t end_idx = m_message.find_first_of(" \r\n", space_idx + 1);
	std::string filename { m_message.substr(space_idx + 1, end_idx - space_idx - 1) };

	if ( filename.find("..") != std::string::npos || filename.find("/") != std::string::npos )
	{
		m_responseHandler->sendResponse(m_username, m_channel, "");
		m_responseHandler->sendResponse(m_username, m_channel,
			COLOR RED "Invalid filename." RESET);
		m_responseHandler->sendResponse(m_username, m_channel, "");
		return;
	}

	std::string filepath { "Bot/bot_files/" + filename };
	std::ifstream file(filepath);

	if ( !file.is_open() )
	{
		m_responseHandler->sendResponse(m_username, m_channel, "");
		m_responseHandler->sendResponse(m_username, m_channel,
			"File " COLOR RED "'" + filename + "'" RESET " not found.");
		m_responseHandler->sendResponse(m_username, m_channel,
			"Type " COLOR YELLOW "!files" RESET " to see available files.");
		m_responseHandler->sendResponse(m_username, m_channel, "");
		return;
	}

	if ( !m_channel.empty() )
		m_responseHandler->sendResponse(m_username, m_channel,
			"File " COLOR GREEN "'" + filename + "'" RESET " sent to your DMs, " COLOR RED + m_username + RESET ".");

	m_responseHandler->sendResponse(m_username, "", "");
	m_responseHandler->sendResponse(m_username, "",
		COLOR LIGHT_CYAN "=== " + filename + " ===" RESET);
	m_responseHandler->sendResponse(m_username, "", "");

	std::string line;
	while ( std::getline(file, line) )
		m_responseHandler->sendResponse(m_username, "", line);

	m_responseHandler->sendResponse(m_username, "", "");
	m_responseHandler->sendResponse(m_username, "",
		COLOR LIGHT_CYAN "=== End of " + filename + " ===" RESET);
	m_responseHandler->sendResponse(m_username, "", "");

	file.close();
}
