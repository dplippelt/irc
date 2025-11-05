/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Commands.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/30 17:16:17 by spyun         #+#    #+#                 */
/*   Updated: 2025/11/05 11:03:18 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Commands.hpp"
#include <cstring>

Commands::Commands(std::map<int, User*>& users,
				   std::map<std::string, Channel*>& channels,
				   const std::string& password)
	: _users(users)
	, _channels(channels)
	, _serverPassword(password)
{
	(void)_channels;
}

Commands::~Commands() {}

// ==================== Helper Functions ====================

void Commands::sendResponse(int fd, const std::string& message)
{
	std::string fullMessage = message;
	if (fullMessage.empty() || fullMessage.back() != '\n')
		fullMessage += "\r\n";

	send(fd, fullMessage.c_str(), fullMessage.length(), 0);

	#ifdef DEBUG
	std::cout << "Sent to fd " << fd << ": " << fullMessage;
	#endif

	ssize_t sent = send(fd, fullMessage.c_str(), fullMessage.length(), 0);
	if (sent == -1)
	{
		std::cerr << "Error: Failed to send message to fd " << fd << ": "
				  << strerror(errno) << std::endl;
	}
}

void Commands::sendNumericReply(int fd, int code, const std::string& message)
{
	std::ostringstream oss;
	oss << ":ft_irc " << code << " * " << message;
	sendResponse(fd, oss.str());
}

void Commands::sendError(int fd, const std::string& command, const std::string& message)
{
	std::string errorMsg = "ERROR :" + command + " " + message;
	sendResponse(fd, errorMsg);
}

bool Commands::isValidNickname(const std::string& nick) const
{
	// IRC nickname rules:
	// - Must start with a letter
	// - Can contain letters, numbers, and special chars: - [ ] \ ` ^ { }
	// - Length between 1 and 9 characters (we'll use 1-20 for flexibility)

	if (nick.empty() || nick.length() > 20)
		return false;

	// First character must be a letter
	if (!std::isalpha(static_cast<unsigned char>(nick[0])))
		return false;

	// Rest can be alphanumeric or special chars
	for (size_t i = 1; i < nick.length(); ++i)
	{
		char c = nick[i];
		if (!std::isalnum(static_cast<unsigned char>(c))
			&& c != '-' && c != '[' && c != ']' && c != '\\'
			&& c != '`' && c != '^' && c != '{' && c != '}'
			&& c != '_')
			return false;
	}

	return true;
}

bool Commands::isNicknameInUse(const std::string& nick) const
{
	for (std::map<int, User*>::const_iterator it = _users.begin();
		 it != _users.end(); ++it)
	{
		if (it->second->getNickname() == nick)
			return true;
	}
	return false;
}

void Commands::checkRegistration(User* user)
{
	// User is registered when:
	// 1. Authenticated (PASS command)
	// 2. Has nickname (NICK command)
	// 3. Has username (USER command)

	if (user->isAuthenticated()
		&& !user->getNickname().empty()
		&& !user->getUsername().empty()
		&& !user->isRegistered())
	{
		user->setRegistered(true);
		sendWelcome(user);
	}
}

void Commands::sendWelcome(User* user)
{
	std::string nick = user->getNickname();
	std::string prefix = user->getPrefix();

	// RPL_WELCOME (001)
	std::ostringstream msg1;
	msg1 << ":ft_irc 001 " << nick << " :Welcome to the Internet Relay Network " << prefix;
	sendResponse(user->getFd(), msg1.str());

	// RPL_YOURHOST (002)
	std::ostringstream msg2;
	msg2 << ":ft_irc 002 " << nick << " :Your host is ft_irc, running version 1.0";
	sendResponse(user->getFd(), msg2.str());

	// RPL_CREATED (003)
	std::ostringstream msg3;
	msg3 << ":ft_irc 003 " << nick << " :This server was created sometime";
	sendResponse(user->getFd(), msg3.str());

	// RPL_MYINFO (004)
	std::ostringstream msg4;
	msg4 << ":ft_irc 004 " << nick << " ft_irc 1.0 o itkol";
	sendResponse(user->getFd(), msg4.str());
}

// ==================== Command Dispatcher ====================

void Commands::executeCommand(User* user, const std::string& command,
							  const std::vector<std::string>& params)
{
	if (command == "PASS")
		handlePASS(user, params);
	else if (command == "NICK")
		handleNICK(user, params);
	else if (command == "USER")
		handleUSER(user, params);
	// Add more commands here as they are implemented
	else
	{
		sendNumericReply(user->getFd(), 421, command + " :Unknown command");
	}
}

// ==================== Authentication Commands ====================

void Commands::handlePASS(User* user, const std::vector<std::string>& params)
{
	// Check if already registered
	if (user->isRegistered())
	{
		sendNumericReply(user->getFd(), ERR_ALREADYREGISTRED,
						":You may not reregister");
		return;
	}

	// Check parameters
	if (params.empty())
	{
		sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS,
						"PASS :Not enough parameters");
		return;
	}

	// Verify password
	std::string providedPassword = params[0];
	// Remove leading ':' if present
	if (!providedPassword.empty() && providedPassword[0] == ':')
		providedPassword = providedPassword.substr(1);

	if (providedPassword != _serverPassword)
	{
		sendNumericReply(user->getFd(), ERR_PASSWDMISMATCH,
						":Password incorrect");
		return;
	}

	// Password correct
	user->setAuthenticated(true);

	// Check if registration is now complete
	checkRegistration(user);
}

void Commands::handleNICK(User* user, const std::vector<std::string>& params)
{
	// Check parameters
	if (params.empty())
	{
		sendNumericReply(user->getFd(), ERR_NONICKNAMEGIVEN,
						":No nickname given");
		return;
	}

	std::string newNick = params[0];
	// Remove leading ':' if present
	if (!newNick.empty() && newNick[0] == ':')
		newNick = newNick.substr(1);

	// Validate nickname format
	if (!isValidNickname(newNick))
	{
		sendNumericReply(user->getFd(), ERR_ERRONEUSNICKNAME,
						newNick + " :Erroneous nickname");
		return;
	}

	// Check if nickname is already in use
	if (isNicknameInUse(newNick))
	{
		sendNumericReply(user->getFd(), ERR_NICKNAMEINUSE,
						newNick + " :Nickname is already in use");
		return;
	}

	// Set the nickname
	std::string oldNick = user->getNickname();
	user->setNickname(newNick);

	// If user is already registered, notify about nick change
	if (user->isRegistered())
	{
		std::string nickChangeMsg = user->getPrefix() + " NICK :" + newNick;
		sendResponse(user->getFd(), nickChangeMsg);
	}

	// Check if registration is now complete
	checkRegistration(user);
}

void Commands::handleUSER(User* user, const std::vector<std::string>& params)
{
	// Check if already registered
	if (user->isRegistered())
	{
		sendNumericReply(user->getFd(), ERR_ALREADYREGISTRED,
						":You may not reregister");
		return;
	}

	// USER command requires 4 parameters:
	// USER <username> <hostname> <servername> :<realname>
	if (params.size() < 4)
	{
		sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS,
						"USER :Not enough parameters");
		return;
	}

	std::string username = params[0];
	// params[1] and params[2] are hostname and servername (ignored in modern IRC)
	std::string realname = params[3];

	// Remove leading ':' from realname if present
	if (!realname.empty() && realname[0] == ':')
		realname = realname.substr(1);

	// Set user information
	user->setUsername(username);
	user->setRealname(realname);
	user->setHostname("localhost"); // You can get actual hostname if needed

	// Check if registration is now complete
	checkRegistration(user);
}
