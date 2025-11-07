/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Commands.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/30 17:16:17 by spyun         #+#    #+#                 */
/*   Updated: 2025/11/07 10:05:56 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Commands.hpp"

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
	oss << ":ft_irc " << std::setw(3) << std::setfill('0') << code << " * " << message;
	sendResponse(fd, oss.str());

	#ifdef DEBUG
	std::cout << "Numeric reply " << code << " sent to fd " << fd << std::endl;
	#endif
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

bool Commands::isValidChannelName(const std::string& channelName) const
{
	// IRC channel names must:
	// - Start with # or &
	// - Be between 2 and 50 characters
	// - Not contain spaces, commas, or control characters

	if (channelName.length() < 2 || channelName.length() > 50)
		return false;
	if (channelName[0] != '#' && channelName[0] != '&')
		return false;

	for (size_t i = 1; i < channelName.length(); ++i)
	{
		char c = channelName[i];
		if (std::isspace(static_cast<unsigned char>(c)) || c == ',' || std::iscntrl(static_cast<unsigned char>(c)))
			return false;
	}
	return true;
}

Channel* Commands::getOrCreateChannel(const std::string& channelName)
{
	std::map<std::string, Channel*>::iterator it = _channels.find(channelName);
	if (it != _channels.end())
		return it->second;

	// Create new channel
	Channel* newChannel = new Channel(channelName);
	_channels[channelName] = newChannel;

	#ifdef DEBUG
	std::cout << "Created new channel: " << channelName << std::endl;
	#endif

	return newChannel;
}

void Commands::sendJoinMessages(User* user, Channel* channel)
{
	std::string joinMsg = user->getPrefix() + " JOIN " + channel->getName();

	// send JOIN msg to all members including the user
	const std::map<int, User*>& members = channel->getMembers();
	for (std::map<int, User*>::const_iterator it = members.begin();
		 it != members.end(); ++it)
	{
		sendResponse(it->second->getFd(), joinMsg);
	}

	// send topic if exists
	if (!channel->getTopic().empty())
	{
		std::ostringstream topicMsg;
		topicMsg << ":ft_irc " << std::setw(3) << std::setfill('0') << RPL_TOPIC
				 << " " << user->getNickname() << " "
				 << channel->getName() << " :" << channel->getTopic();
		sendResponse(user->getFd(), topicMsg.str());
	}

	// send names list (RPL_NAMREPLY)
	std::ostringstream namesMsg;
	namesMsg << ":ft_irc " << std::setw(3) << std::setfill('0') << RPL_NAMREPLY
			 << " " << user->getNickname() << " = " << channel->getName() << " :"
			 << channel->getMemberList();
	sendResponse(user->getFd(), namesMsg.str());

	// send end of names list (RPL_ENDOFNAMES)
	std::ostringstream endNamesMsg;
	endNamesMsg << ":ft_irc " << std::setw(3) << std::setfill('0') << RPL_ENDOFNAMES
				<< " " << user->getNickname() << " "
				<< channel->getName() << " :End of /NAMES list";
	sendResponse(user->getFd(), endNamesMsg.str());
}

void Commands::checkRegistration(User* user)
{
	// User is registered when:
	// 1. Authenticated (PASS command)
	// 2. Has nickname (NICK command)
	// 3. Has username (USER command)
	// 4. Not already registered

	if (user->hasProvidedPassword()
		&& user->hasNickname()
		&& user->hasUsername()
		&& !user->isRegistered())
	{
		user->setRegistered(true);
		sendWelcome(user);

		#ifdef DEBUG
		std::cout << "User " << user->getNickname()
					<< " (fd " << user->getFd()
					<< ") is now registered!" << std::endl;
        #endif
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

bool Commands::canExecuteCommand(User* user, const std::string& command) const
{
	// PASS always allowed
	if (command == "PASS")
		return true;

	// NICK and USER only after PASS
	if (command == "NICK" || command == "USER")
	{
		if (!user->hasProvidedPassword())
			return false;
		return true;
	}

	// Other commands only after registration
	if (!user->isRegistered())
		return false;

	return true;
}

void Commands::sendAuthenticationError(int fd, const std::string& command)
{
	if (command == "NICK" || command == "USER")
		sendNumericReply(fd, ERR_NOTREGISTERED, ":You have not registered");
	else
		sendNumericReply(fd, ERR_NOTREGISTERED, ":You have not registered");
}

// ==================== Temporary Wrappers ====================

void Commands::executeCommand(User* user, const std::string& command,
								const std::vector<std::string>& params)
{
	if (!canExecuteCommand(user, command))
	{
		sendAuthenticationError(user->getFd(), command);
		return;
	}

	std::list<std::string> paramList(params.begin(), params.end());

	if (command == "PASS")
		handlePASS(user, paramList);
	else if (command == "NICK")
		handleNICK(user, paramList);
	else if (command == "USER")
		handleUSER(user, paramList);
	else if (command == "JOIN")
		handleJOIN(user, paramList);
	else if (command == "PRIVMSG")
		handlePRIVMSG(user, paramList);
	else
	{
		sendNumericReply(user->getFd(), 421, command + " :Unknown command");
	}
}

// ==================== Authentication Commands ====================

void Commands::handlePASS(User* user, const std::list<std::string>& params)
{
	// Check if already registered
	if (user->isRegistered())
	{
		sendNumericReply(user->getFd(), ERR_ALREADYREGISTRED, ":You may not reregister");
		return;
	}

	if (user->hasProvidedPassword())
	{
		sendNumericReply(user->getFd(), ERR_ALREADYREGISTRED, ":You have already provided a password");
		return;
	}

	// Check parameters
	if (params.empty())
	{
		sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "PASS :Not enough parameters");
		return;
	}

	// Verify password
	std::string providedPassword = params.front();
	// Remove leading ':' if present
	if (!providedPassword.empty() && providedPassword[0] == ':')
		providedPassword = providedPassword.substr(1);

	if (providedPassword != _serverPassword)
	{
		sendNumericReply(user->getFd(), ERR_PASSWDMISMATCH,":Password incorrect");
		return;
	}

	// Password correct
	user->setPasswordProvided(true);
	user->setAuthenticated(true);

	#ifdef DEBUG
	std::cout << "User fd " << user->getFd() << " provided correct password." << std::endl;
	#endif
}

void Commands::handleNICK(User* user, const std::list<std::string>& params)
{
	// Check parameters
	if (params.empty())
	{
		sendNumericReply(user->getFd(), ERR_NONICKNAMEGIVEN, ":No nickname given");
		return;
	}

	std::string newNick = params.front();
	// Remove leading ':' if present
	if (!newNick.empty() && newNick[0] == ':')
		newNick = newNick.substr(1);

	// Validate nickname format
	if (!isValidNickname(newNick))
	{
		sendNumericReply(user->getFd(), ERR_ERRONEUSNICKNAME, newNick + " :Erroneous nickname");
		return;
	}

	// Check if nickname is already in use
	if (isNicknameInUse(newNick))
	{
		sendNumericReply(user->getFd(), ERR_NICKNAMEINUSE, newNick + " :Nickname is already in use");
		return;
	}

	// Set the nickname
	std::string oldNick = user->getNickname();
	user->setNickname(newNick);
	user->setHasNickname(true);

	// If user is already registered, notify about nick change
	if (user->isRegistered())
	{
		std::string nickChangeMsg = user->getPrefix() + " NICK :" + newNick;
		sendResponse(user->getFd(), nickChangeMsg);
	}

	// Check if registration is now complete
	checkRegistration(user);

	#ifdef DEBUG
	std::cout << "User fd" << user->getFd() << " set nickname to " << newNick << std::endl;
	#endif
}

void Commands::handleUSER(User* user, const std::list<std::string>& params)
{
	// Check if already registered
	if (user->isRegistered())
	{
		sendNumericReply(user->getFd(), ERR_ALREADYREGISTRED, ":You may not reregister");
		return;
	}

	if (user->hasUsername())
	{
		sendNumericReply(user->getFd(), ERR_ALREADYREGISTRED, ":You have already set a username");
		return;
	}

	// USER command requires 4 parameters:
	// USER <username> <hostname> <servername> :<realname>
	if (params.size() < 4)
	{
		sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "USER :Not enough parameters");
		return;
	}

	std::list<std::string>::const_iterator it = params.begin();
	std::string username = *it++;
	std::advance(it, 2); // Skip hostname and servername
	std::string realname = *it;

	if (!realname.empty() && realname[0] == ':')
		realname = realname.substr(1);

	// Set user information
	user->setUsername(username);
	user->setRealname(realname);
	user->setHostname("localhost"); // can get actual hostname if needed
	user->setHasUsername(true);

	// Check if registration is now complete
	checkRegistration(user);

	#ifdef DEBUG
	std::cout << "User fd " << user->getFd() << " set username to " << username << std::endl;
	#endif
}

void Commands::handleJOIN(User* user, const std::list<std::string>& params)
{
	//check if user is registered
	if (!user->isRegistered())
	{
		sendNumericReply(user->getFd(), ERR_NOTREGISTERED,
						":You have not registered");
		return;
	}

	// Check parameters
	if (params.empty())
	{
		sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS,
						"JOIN :Not enough parameters");
		return;
	}

	// parse channel names (comma separated)
	std::string channelList = params.front();
	// Remove leading ':' if present
	if (!channelList.empty() && channelList[0] == ':')
		channelList = channelList.substr(1);

	// Parse keys if provided
	std::string keyList;
	if (params.size() > 1)
	{
		std::list<std::string>::const_iterator it = params.begin();
		++it;
		keyList = *it;
		if (!keyList.empty() && keyList[0] == ':')
			keyList = keyList.substr(1);
	}

	// Split channels and keys by comma
	std::vector<std::string> channels;
	std::vector<std::string> keys;

	std::istringstream channelStream(channelList);
	std::string channe;
	while (std::getline(channelStream, channe, ','))
	{
		if (!channe.empty())
			channels.push_back(channe);
	}

	if (!keyList.empty())
	{
		std::istringstream keyStream(keyList);
		std::string key;
		while (std::getline(keyStream, key, ','))
		{
			keys.push_back(key);
		}
	}

	// Process each channel
	for (size_t i = 0; i < channels.size(); ++i)
	{
		std::string channelName = channels[i];
		std::string channelKey = (i < keys.size()) ? keys[i] : "";

		// Validate channel name
		if (!isValidChannelName(channelName))
		{
			sendNumericReply(user->getFd(), ERR_NOSUCHCHANNEL,
							channelName + " :No such channel");
			continue;
		}

		// Get or create channel
		Channel* channel = getOrCreateChannel(channelName);

		// Check if user is already in the channel
		if (channel->isMember(user->getFd()))
		{
			continue;
		}

		//check channel modes and restrictions
		// Invite-only
		if (channel->isInviteOnly() && !channel->isInvited(user->getFd()))
		{
			sendNumericReply(user->getFd(), ERR_INVITEONLYCHAN,
							channelName + " :Cannot join channel (+i)");
			continue;
		}
		// Check if channel has a key (password)
		if (channel->hasKey())
		{
			if (channelKey.empty() || channelKey != channel->getKey())
			{
				sendNumericReply(user->getFd(), ERR_BADCHANNELKEY,
								channelName + " :Cannot join channel (+k)");
				continue;
			}
		}
		// Check user limit
		if (channel->hasUserLimit())
		{
			if (static_cast<int>(channel->getMemberCount()) >= channel->getUserLimit())
			{
				sendNumericReply(user->getFd(), ERR_CHANNELISFULL,
								channelName + " :Cannot join channel (+l)");
				continue;
			}
		}

		// add user to channel
		channel->addMember(user);
		user->joinChannel(channelName);

		// Remove from invite list if they were invited
		if (channel->isInvited(user->getFd()))
			channel->removeInvite(user->getFd());

		// Send JOIN messages
		sendJoinMessages(user, channel);

		#ifdef DEBUG
		std::cout << "User " << user->getNickname()
				  << " joined channel " << channelName << std::endl;
		#endif
	}
}

void Commands::handlePRIVMSG(User* user, const std::list<std::string>& params)
{
	// Check if user is registered
	if (!user->isRegistered())
	{
		sendNumericReply(user->getFd(), ERR_NOTREGISTERED, ":You have not registered");
		return;
	}
	// PRIVMSG requires at least 2 parameters: <target> :<message>
	if (params.empty())
	{
		sendNumericReply(user->getFd(), ERR_NORECIPIENT, "PRIVMSG :No recipient given");
		return;
	}
	if (params.size() < 2)
	{
		sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "PRIVMSG :Not enough parameters");
		return;
	}

	std::list<std::string>::const_iterator it = params.begin();
	std::string target = *it++;
	std::string message = *it;

	// Remove leading ':' from message if present
	if (!message.empty() && message[0] == ':')
		message = message.substr(1);

	for (; it != params.end(); ++it)
		message += " " + *it;

	// check if target is a channel (starts with # or &)
	if (target[0] == '#' || target[0] == '&')
	{
		std::map<std::string, Channel*>::iterator it = _channels.find(target);
		if (it == _channels.end())
		{
			sendNumericReply(user->getFd(), ERR_NOSUCHCHANNEL, target + " :No such channel");
			return;
		}

		Channel* channel = it->second;

		// Check if user is a member of the channel
		if (!channel->isMember(user->getFd()))
		{
			sendNumericReply(user->getFd(), ERR_CANNOTSENDTOCHAN, target + " :Cannot send to channel");
			return;
		}

		// Construct and send message to all channel members except sender
		std::string privmsgToChannel = user->getPrefix() + " PRIVMSG " + target + " :" + message;

		const std::map<int, User*>& members = channel->getMembers();
		for (std::map<int, User*>::const_iterator it = members.begin();
			 it != members.end(); ++it)
		{
			if (it->first != user->getFd()) // Dont send to sender
				sendResponse(it->second->getFd(), privmsgToChannel);
		}

		#ifdef DEBUG
		std::cout << "User " << user->getNickname()
				  << " sent message to channel " << target << std::endl;
		#endif
	}
	else
	{
		// send to user (direct message)
		User* targetUser = nullptr;

		//find user by nickname
		for (std::map<int, User*>::iterator it = _users.begin();
			 it != _users.end(); ++it)
		{
			if (it->second->getNickname() == target)
			{
				targetUser = it->second;
				break;
			}
		}

		if (targetUser == nullptr)
		{
			sendNumericReply(user->getFd(), ERR_NOSUCHNICK, target + " :No such nick/channel");
			return;
		}

		// Construct and send message
		std::string privmsgToUser = user->getPrefix() + " PRIVMSG " + target + " :" + message;
		sendResponse(targetUser->getFd(), privmsgToUser);

		#ifdef DEBUG
		std::cout << "User " << user->getNickname()
				  << " sent DM to " << target << ": " << message << std::endl;
		#endif

	}

}
