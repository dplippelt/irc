/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 17:16:17 by spyun             #+#    #+#             */
/*   Updated: 2025/11/17 15:04:00 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Commands.hpp"
#include "Server.hpp"
#include "Message.hpp"

Commands::Commands(std::map<int, User*>& users,
				   std::map<std::string, Channel*>& channels,
				   const std::string& password)
	: _users(users)
	, _channels(channels)
	, _serverPassword(password)
{}

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

// bool Commands::isValidNickname(const std::string& nick) const
// {
// 	// IRC nickname rules:
// 	// - Must start with a letter
// 	// - Can contain letters, numbers, and special chars: - [ ] \ ` ^ { }
// 	// - Length between 1 and 9 characters (we'll use 1-20 for flexibility)

// 	if (nick.empty() || nick.length() > 20)
// 		return false;
// 	if (!std::isalpha(static_cast<unsigned char>(nick[0])))
// 		return false;

// 	for (size_t i = 1; i < nick.length(); ++i)
// 	{
// 		char c = nick[i];
// 		if (!std::isalnum(static_cast<unsigned char>(c))
// 			&& c != '-' && c != '[' && c != ']' && c != '\\'
// 			&& c != '`' && c != '^' && c != '{' && c != '}'
// 			&& c != '_')
// 			return false;
// 	}

// 	return true;
// }

// bool Commands::isNicknameInUse(const std::string& nick) const
// {
// 	for (std::map<int, User*>::const_iterator it = _users.begin();
// 		 it != _users.end(); ++it)
// 	{
// 		if (it->second->getNickname() == nick)
// 			return true;
// 	}
// 	return false;
// }

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

	const std::map<int, User*>& members = channel->getMembers();
	for (std::map<int, User*>::const_iterator it = members.begin();
		 it != members.end(); ++it)
	{
		sendResponse(it->second->getFd(), joinMsg);
	}

	if (!channel->getTopic().empty())
	{
		std::ostringstream topicMsg;
		topicMsg << ":ft_irc " << std::setw(3) << std::setfill('0') << RPL_TOPIC
				 << " " << user->getNickname() << " "
				 << channel->getName() << " :" << channel->getTopic();
		sendResponse(user->getFd(), topicMsg.str());
	}

	std::ostringstream namesMsg;
	namesMsg << ":ft_irc " << std::setw(3) << std::setfill('0') << RPL_NAMREPLY
			 << " " << user->getNickname() << " = " << channel->getName() << " :"
			 << channel->getMemberList();
	sendResponse(user->getFd(), namesMsg.str());

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
	if (command == "PASS")
		return true;
	if (command == "NICK" || command == "USER")
	{
		if (!user->hasProvidedPassword())
			return false;
		return true;
	}
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
								const std::vector<std::string>& params, const Server& server)
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
		handleNICK(user, paramList, server);
	else if (command == "USER")
		handleUSER(user, paramList);
	else if (command == "JOIN")
		handleJOIN(user, paramList);
	else if (command == "PRIVMSG")
		handlePRIVMSG(user, paramList, server);
	else if (command == "KICK")
		handleKICK(user, paramList, server);
	else if (command == "PART")
		handlePART(user, paramList, server);
	else if (command == "TOPIC")
		handleTOPIC(user, paramList, server);
	else if (command == "INVITE")
		handleINVITE(user, paramList, server);
	else
	{
		sendNumericReply(user->getFd(), 421, command + " :Unknown command");
	}
}

void Commands::handlePASS(User* user, const std::list<std::string>& params)
{
	// if (user->isRegistered())
	// {
	// 	sendNumericReply(user->getFd(), ERR_ALREADYREGISTRED, ":You may not reregister");
	// 	return;
	// }
	// if (user->hasProvidedPassword())
	// {
	// 	sendNumericReply(user->getFd(), ERR_ALREADYREGISTRED, ":You have already provided a password");
	// 	return;
	// }
	// if (params.empty())
	// {
	// 	sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "PASS :Not enough parameters");
	// 	return;
	// }

	// Dominique: start new validation
	try
	{
		Validation::validatePASS(user, params);
	}
	catch ( const std::exception& e )
	{
		return;
	}
	// Dominique: end new validation

	std::string providedPassword = params.front();

	if (!providedPassword.empty() && providedPassword[0] == ':')
		providedPassword = providedPassword.substr(1);
	if (providedPassword != _serverPassword)
	{
		sendNumericReply(user->getFd(), ERR_PASSWDMISMATCH,":Password incorrect");
		return;
	}

	user->setPasswordProvided(true);
	user->setAuthenticated(true);

	checkRegistration(user);

	#ifdef DEBUG
	std::cout << "User fd " << user->getFd() << " provided correct password." << std::endl;
	#endif
}

void Commands::handleNICK(User* user, const std::list<std::string>& params, const Server& server)
{
	// if (params.empty())
	// {
	// 	sendNumericReply(user->getFd(), ERR_NONICKNAMEGIVEN, ":No nickname given");
	// 	return;
	// }

	// std::string newNick = params.front();
	// if (!newNick.empty() && newNick[0] == ':')
	// 	newNick = newNick.substr(1);
	// if (!isValidNickname(newNick))
	// {
	// 	sendNumericReply(user->getFd(), ERR_ERRONEUSNICKNAME, newNick + " :Erroneous nickname");
	// 	return;
	// }
	// if (isNicknameInUse(newNick))
	// {
	// 	sendNumericReply(user->getFd(), ERR_NICKNAMEINUSE, newNick + " :Nickname is already in use");
	// 	return;
	// }

	// Dominique: start new validation
	std::string newNick {};

	try
	{
		newNick = Validation::validateNICK(user, params, server);
	}
	catch(const std::exception& e)
	{
		return;
	}
	// Dominique: end new validation

	std::string oldNick = user->getNickname();
	user->setNickname(newNick);
	user->setHasNickname(true);

	if (user->isRegistered())
	{
		std::string nickChangeMsg = user->getPrefix() + " NICK :" + newNick;
		sendResponse(user->getFd(), nickChangeMsg);
	}

	checkRegistration(user);

	#ifdef DEBUG
	std::cout << "User fd" << user->getFd() << " set nickname to " << newNick << std::endl;
	#endif
}

void Commands::handleUSER(User* user, const std::list<std::string>& params)
{
	// if (user->isRegistered())
	// {
	// 	sendNumericReply(user->getFd(), ERR_ALREADYREGISTRED, ":You may not reregister");
	// 	return;
	// }
	// if (user->hasUsername())
	// {
	// 	sendNumericReply(user->getFd(), ERR_ALREADYREGISTRED, ":You have already set a username");
	// 	return;
	// }
	// if (params.size() < 4)
	// {
	// 	sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "USER :Not enough parameters");
	// 	return;
	// }

	// Dominique: start new validation
	try
	{
		Validation::validateUSER(user, params);
	}
	catch (const std::exception& e)
	{
		return;
	}
	// Dominique: end new validation

	std::list<std::string>::const_iterator it = params.begin();
	std::string username = *it++;
	std::advance(it, 2);
	std::string realname = *it;

	if (!realname.empty() && realname[0] == ':')
		realname = realname.substr(1);

	user->setUsername(username);
	user->setRealname(realname);
	user->setHostname("localhost");
	user->setHasUsername(true);

	checkRegistration(user);

	#ifdef DEBUG
	std::cout << "User fd " << user->getFd() << " set username to " << username << std::endl;
	#endif
}

void Commands::handleJOIN(User* user, const std::list<std::string>& params)
{
	// if (!user->isRegistered())
	// {
	// 	sendNumericReply(user->getFd(), ERR_NOTREGISTERED, ":You have not registered");
	// 	return;
	// }
	// if (params.empty())
	// {
	// 	sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "JOIN :Not enough parameters");
	// 	return;
	// }

	// Dominique: start new validation
	try
	{
		Validation::validateJOIN(user, params);
	}
	catch(const std::exception& e)
	{
		return;
	}
	// Dominique: end new validation

	std::string channelList = params.front();
	if (!channelList.empty() && channelList[0] == ':')
		channelList = channelList.substr(1);

	std::string keyList;
	if (params.size() > 1)
	{
		std::list<std::string>::const_iterator it = params.begin();
		++it;
		keyList = *it;
		if (!keyList.empty() && keyList[0] == ':')
			keyList = keyList.substr(1);
	}

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


	for (size_t i = 0; i < channels.size(); ++i)
	{
		std::string channelName = channels[i];
		std::string channelKey = (i < keys.size()) ? keys[i] : "";

		if (!isValidChannelName(channelName))
		{
			sendNumericReply(user->getFd(), ERR_NOSUCHCHANNEL, channelName + " :No such channel");
			continue;
		}

		Channel* channel = getOrCreateChannel(channelName);

		// if (channel->isMember(user->getFd()))
		// 	continue;

		// if (channel->isInviteOnly() && !channel->isInvited(user->getFd()))
		// {
		// 	sendNumericReply(user->getFd(), ERR_INVITEONLYCHAN, channelName + " :Cannot join channel (+i)");
		// 	continue;
		// }
		// if (channel->hasKey())
		// {
		// 	if (channelKey.empty() || channelKey != channel->getKey())
		// 	{
		// 		sendNumericReply(user->getFd(), ERR_BADCHANNELKEY, channelName + " :Cannot join channel (+k)");
		// 		continue;
		// 	}
		// }
		// if (channel->hasUserLimit())
		// {
		// 	if (static_cast<int>(channel->getMemberCount()) >= channel->getUserLimit())
		// 	{
		// 		sendNumericReply(user->getFd(), ERR_CHANNELISFULL, channelName + " :Cannot join channel (+l)");
		// 		continue;
		// 	}
		// }

		// Dominique: start new 'validation'
		try
		{
			Validation::validateCanJoin(user, channel, channelKey);
		}
		catch(const std::exception& e)
		{
			continue;
		}
		// Dominique: end new 'validation'

		channel->addMember(user);
		user->joinChannel(channelName);

		if (channel->isInvited(user->getFd()))
			channel->removeInvite(user->getFd());

		sendJoinMessages(user, channel);

		#ifdef DEBUG
		std::cout << "User " << user->getNickname()
				  << " joined channel " << channelName << std::endl;
		#endif
	}
}

void Commands::handlePRIVMSG(User* user, const std::list<std::string>& params, const Server& server)
{
	// if (!user->isRegistered())
	// {
	// 	sendNumericReply(user->getFd(), ERR_NOTREGISTERED, ":You have not registered");
	// 	return;
	// }
	// if (params.empty())
	// {
	// 	sendNumericReply(user->getFd(), ERR_NORECIPIENT, "PRIVMSG :No recipient given");
	// 	return;
	// }
	// if (params.size() < 2)
	// {
	// 	sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "PRIVMSG :Not enough parameters");
	// 	return;
	// }

	// Dominique: start new validation
	try
	{
		Validation::validatePRIVMSG(user, params);
	}
	catch(const std::exception& e)
	{
		return;
	}
	// Dominique: end new validation

	std::list<std::string>::const_iterator it = params.begin();
	std::string target = *it++;
	std::string message = *it;

	if (!message.empty() && message[0] == ':')
		message = message.substr(1);

	for (; it != params.end(); ++it)
		message += " " + *it;

	if (target[0] == '#' || target[0] == '&')
	{
		// std::map<std::string, Channel*>::iterator it = _channels.find(target);
		// if (it == _channels.end())
		// {
		// 	sendNumericReply(user->getFd(), ERR_NOSUCHCHANNEL, target + " :No such channel");
		// 	return;
		// }

		// Channel* channel = it->second;

		// if (!channel->isMember(user->getFd()))
		// {
		// 	sendNumericReply(user->getFd(), ERR_CANNOTSENDTOCHAN, target + " :Cannot send to channel");
		// 	return;
		// }

		// Dominique: start new 'validation'
		Channel* channel {};

		try
		{
			channel = Validation::validateCanSendMsg(user, target, server);
		}
		catch(const std::exception& e)
		{
			return;
		}
		// Dominique: end new 'validation'

		std::string privmsgToChannel = user->getPrefix() + " PRIVMSG " + target + " :" + message;

		const std::map<int, User*>& members = channel->getMembers();
		for (std::map<int, User*>::const_iterator it = members.begin();
			 it != members.end(); ++it)
		{
			if (it->first != user->getFd())
				sendResponse(it->second->getFd(), privmsgToChannel);
		}

		#ifdef DEBUG
		std::cout << "User " << user->getNickname()
				  << " sent message to channel " << target << std::endl;
		#endif
	}
	else
	{
		User* targetUser = nullptr;

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
			sendNumericReply(user->getFd(), ERR_NOSUCHNICK, target + " :No such nick");
			return;
		}

		std::string privmsgToUser = user->getPrefix() + " PRIVMSG " + target + " :" + message;
		sendResponse(targetUser->getFd(), privmsgToUser);

		#ifdef DEBUG
		std::cout << "User " << user->getNickname()
				  << " sent DM to " << target << ": " << message << std::endl;
		#endif

	}

}

void Commands::handleKICK(User* user, const std::list<std::string>& params, const Server& server)
{
	// if (!user->isRegistered())
	// {
	// 	sendNumericReply(user->getFd(), ERR_NOTREGISTERED, ":You have not registered");
	// 	return;
	// }
	// if (params.size() < 2)
	// {
	// 	sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "KICK :Not enough parameters");
	// 	return;
	// }

	// std::list<std::string>::const_iterator it = params.begin();
	// std::string channelName = *it++;
	// std::string targetNick = *it++;

	// std::string reason = "Kicked by operator";
	// if (it != params.end())
	// {
	// 	reason = *it;
	// 	if (!reason.empty() && reason[0] == ':')
	// 		reason = reason.substr(1);
	// }

	// std::map<std::string, Channel*>::iterator chanIt = _channels.find(channelName);
	// if (chanIt == _channels.end())
	// {
	// 	sendNumericReply(user->getFd(), ERR_NOSUCHCHANNEL, channelName + " :No such channel");
	// 	return;
	// }

	// Channel* channel = chanIt->second;

	// if (!channel->isMember(user->getFd()))
	// {
	// 	sendNumericReply(user->getFd(), ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
	// 	return;
	// }
	// if (!channel->isOperator(user->getFd()))
	// {
	// 	sendNumericReply(user->getFd(), ERROR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
	// 	return;
	// }

	// User* targetUser = nullptr;
	// for (std::map<int, User*>::iterator it = _users.begin(); it != _users.end(); ++it)
	// {
	// 	if (it->second->getNickname() == targetNick)
	// 	{
	// 		targetUser = it->second;
	// 		break;
	// 	}
	// }

	// if (!channel->isMember(targetUser->getFd()))
	// {
	// 	sendNumericReply(user->getFd(), ERR_USERNOTINCHANNEL, targetNick + " " + channelName + " :They aren't on that channel");
	// 	return;
	// }

	// Dominique: start new validation
	std::string	targetNick {};
	std::string	channelName {};
	std::string	reason {};
	Channel*	channel {};
	User*		targetUser {};

	try
	{
		Validation::validateKICK(user, params, targetNick, channelName, reason);
		channel = Validation::validateCanKick(user, channelName, server);
		targetUser = Validation::validateCanKickTarget(user, channel, targetNick, server);
	}
	catch(const std::exception& e)
	{
		return;
	}
	// Dominique: end new validation

	std::string kickMsg = user->getPrefix() + " KICK " + channelName + " " + targetNick + " :" + reason;
	const std::map<int, User*>& members = channel->getMembers();
	for (std::map<int, User*>::const_iterator it = members.begin(); it != members.end(); ++it)
		sendResponse(it->second->getFd(), kickMsg);

	channel->removeMember(targetUser->getFd());
	targetUser->leaveChannel(channelName);

	if (channel->isEmpty())
	{
		_channels.erase(channelName);
		delete channel;

		#ifdef DEBUG
		std::cout << "Channel " << channelName << " deleted as it became empty." << std::endl;
		#endif
	}

	#ifdef DEBUG
	std::cout << "User " << targetNick << " was kicked from channel "
			  << channelName << " by " << user->getNickname() << std::endl;
	#endif
}

void Commands::handlePART(User* user, const std::list<std::string>& params, const Server& server)
{
	// PART command removes the user from the given channel.
	// On sending a successful PART command, the user will receive a PART message
	// from the server for each channel they are removed from

	// if (!user->isRegistered())
	// {
	// 	sendNumericReply(user->getFd(), ERR_NOTREGISTERED, ":You have not registered");
	// 	return;
	// }
	// if (params.empty())
	// {
	// 	sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "PART :Not enough parameters");
	// 	return;
	// }

	// Dominique: start new validation
	try
	{
		Validation::validatePART(user, params);
	}
	catch(const std::exception& e)
	{
		return;
	}
	// Dominique: end new validation

	std::string channelList = params.front();
	if (!channelList.empty() && channelList[0] == ':')
		channelList = channelList.substr(1);

	std::string reason;
	if (params.size() > 1)
	{
		std::list<std::string>::const_iterator it = params.begin();
		++it;
		reason = *it;
		if (!reason.empty() && reason[0] == ':')
			reason = reason.substr(1);
		for (++it; it != params.end(); ++it)
			reason += " " + *it;
	}

	std::vector<std::string> channels;
	std::istringstream channelStream(channelList);
	std::string channelName;
	while (std::getline(channelStream, channelName, ','))
	{
		if (!channelName.empty())
			channels.push_back(channelName);
	}

	for (size_t i = 0; i < channels.size(); ++i)
	{
		std::string currentChannel = channels[i];

		// std::map<std::string, Channel*>::iterator chanIt = _channels.find(currentChannel);
		// if (chanIt == _channels.end())
		// {
		// 	sendNumericReply(user->getFd(), ERR_NOSUCHCHANNEL,
		// 					currentChannel + " :No such channel");
		// 	continue;
		// }

		// Channel* channel = chanIt->second;

		// if (!channel->isMember(user->getFd()))
		// {
		// 	sendNumericReply(user->getFd(), ERR_NOTONCHANNEL, currentChannel + " :You're not on that channel");
		// 	continue;
		// }

		// Dominique: start new 'validation'
		Channel* channel {};

		try
		{
			channel = Validation::validateCanPart(user, currentChannel, server);
		}
		catch(const std::exception& e)
		{
			continue;
		}
		// Dominique: end new 'validation'

		std::string partMsg = user->getPrefix() + " PART " + currentChannel;
		if (!reason.empty())
			partMsg += " :" + reason;

		const std::map<int, User*>& members = channel->getMembers();
		for (std::map<int, User*>::const_iterator it = members.begin();
			it != members.end(); ++it)
		{
			sendResponse(it->second->getFd(), partMsg);
		}

		channel->removeMember(user->getFd());
		user->leaveChannel(currentChannel);

		if (channel->isEmpty())
		{
			_channels.erase(currentChannel);
			delete channel;

			#ifdef DEBUG
			std::cout << "Channel " << currentChannel << " deleted as it became empty." << std::endl;
			#endif
		}

		#ifdef DEBUG
		std::cout << "User " << user->getNickname() << " left channel " << currentChannel;
		if (!reason.empty())
			std::cout << " (reason: " << reason << ")";
		std::cout << std::endl;
		#endif
	}

}

void Commands::handleTOPIC(User* user, const std::list<std::string>& params, const Server& server)
{
	// TOPIC command is used to change or view the topic of the given channel.
	// If <topic> is not given, the current topic is returned (or no topic set message)
	// If <topic> is given, the topic is changed (if user has permission)
	// Channel mode +t restricts topic changes to channel operators only

	// if (!user->isRegistered())
	// {
	// 	sendNumericReply(user->getFd(), ERR_NOTREGISTERED, ":You have not registered");
	// 	return;
	// }
	// if (params.empty())
	// {
	// 	sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "TOPIC :Not enough parameters");
	// 	return;
	// }
	// std::string channelName = params.front();
	// if (!channelName.empty() && channelName[0] == ':')
	// 	channelName = channelName.substr(1);

	// std::map<std::string, Channel*>::iterator chanIt = _channels.find(channelName);
	// if (chanIt == _channels.end())
	// {
	// 	sendNumericReply(user->getFd(), ERR_NOSUCHCHANNEL, channelName + " :No such channel");
	// 	return;
	// }

	// Channel* channel = chanIt->second;

	// if (!channel->isMember(user->getFd()))
	// {
	// 	sendNumericReply(user->getFd(), ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
	// 	return;
	// }

	// Dominique: start new validation
	std::string	channelName {};
	Channel*	channel {};

	try
	{
		channelName = Validation::validateTOPIC(user, params);
		channel = Validation::validateCanChangeTopic(user, channelName, server);
	}
	catch(const std::exception& e)
	{
		return;
	}
	// Dominique: start end validation

	if (params.size() == 1)
	{
		const std::string& currentTopic = channel->getTopic();

		if (currentTopic.empty())
		{
			sendNumericReply(user->getFd(), RPL_NOTOPIC, channelName + " :No topic is set"); // Dominique: I've decided to not move this to the validation code as it seems more of a response (just like the else path) than an error message.
		}
		else
		{
			std::ostringstream topicMsg;
			topicMsg << ":ft_irc " << std::setw(3) << std::setfill('0') << RPL_TOPIC
					 << " " << user->getNickname() << " "
					 << channelName << " :" << currentTopic;
			sendResponse(user->getFd(), topicMsg.str());
		}

		#ifdef DEBUG
		std::cout << "User " << user->getNickname() << " viewed topic of " << channelName << std::endl;
		#endif

		return;
	}
	if (channel->isTopicRestricted() && !channel->isOperator(user->getFd()))
	{
		sendNumericReply(user->getFd(), ERROR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
		return;
	}

	std::list<std::string>::const_iterator it = params.begin();
	++it;
	std::string newTopic = *it;

	if (!newTopic.empty() && newTopic[0] == ':')
		newTopic = newTopic.substr(1);

	for (++it; it != params.end(); ++it)
		newTopic += " " + *it;

	channel->setTopic(newTopic);
	std::string topicChangeMsg = user->getPrefix() + " TOPIC " + channelName + " :" + newTopic;

	const std::map<int, User*>& members = channel->getMembers();
	for (std::map<int, User*>::const_iterator memIt = members.begin();
		 memIt != members.end(); ++memIt)
	{
		sendResponse(memIt->second->getFd(), topicChangeMsg);
	}

	#ifdef DEBUG
	std::cout << "User " << user->getNickname()
			  << " changed topic of " << channelName
			  << " to: " << newTopic << std::endl;
	#endif
}

void Commands::handleINVITE(User* user, const std::list<std::string>& params, const Server& server)
{
	// INVITE command invites a user to a channel
	// format: INVITE <nickname> <channel>
	// only channel operators can invite users (when channel is +i)

	// if (!user->isRegistered())
	// {
	// 	sendNumericReply(user->getFd(), ERR_NOTREGISTERED, ":You have not registered");
	// 	return;
	// }
	// if (params.size() < 2)
	// {
	// 	sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "INVITE :Not enough parameters");
	// 	return;
	// }

	// std::list<std::string>::const_iterator it = params.begin();
	// std::string targetNick = *it++;
	// std::string channelName = *it;

	// if (!channelName.empty() && channelName[0] == ':')
	// 	channelName = channelName.substr(1);
	// if (!targetNick.empty() && targetNick[0] == ':')
	// 	targetNick = targetNick.substr(1);

	// std::map<std::string, Channel*>::iterator chanIt = _channels.find(channelName);
	// if (chanIt == _channels.end())
	// {
	// 	sendNumericReply(user->getFd(), ERR_NOSUCHCHANNEL, channelName + " :No such channel");
	// 	return;
	// }

	// Channel* channel = chanIt->second;
	// if (!channel->isMember(user->getFd()))
	// {
	// 	sendNumericReply(user->getFd(), ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
	// 	return;
	// }
	// if (channel->isInviteOnly() && !channel->isOperator(user->getFd()))
	// {
	// 	sendNumericReply(user->getFd(), ERROR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
	// 	return;
	// }

	// User* targetUser = nullptr;
	// for (std::map<int, User*>::iterator it = _users.begin(); it != _users.end(); ++it)
	// {
	// 	if (it->second->getNickname() == targetNick)
	// 	{
	// 		targetUser = it->second;
	// 		break;
	// 	}
	// }
	// if (targetUser == nullptr)
	// {
	// 	sendNumericReply(user->getFd(), ERR_NOSUCHNICK, targetNick + " :No such nick/channel");
	// 	return;
	// }
	// if (channel->isMember(targetUser->getFd()))
	// {
	// 	sendNumericReply(user->getFd(), ERR_USERONCHANNEL, targetNick + " " + channelName + " :is already on channel");
	// 	return;
	// }

	// Dominique: start new validation
	std::string	targetNick {};
	std::string	channelName {};
	Channel*	channel {};
	User*		targetUser {};

	try
	{
		Validation::validateINVITE(user, params, targetNick, channelName);
		channel = Validation::validateCanInvite(user, channelName, server);
		targetUser = Validation::validateCanInviteTarget(user, channel, channelName, targetNick, server);
	}
	catch(const std::exception& e)
	{
		return;
	}
	// Dominique: start end validation

	channel->addInvite(targetUser->getFd());
	std::ostringstream invitingMsg;
	invitingMsg << ":ft_irc " << std::setw(3) << std::setfill('0') << RPL_INVITING
				<< " " << user->getNickname() << " "
				<< targetNick << " " << channelName;
	sendResponse(user->getFd(), invitingMsg.str());
	std::string inviteMsg = user->getPrefix() + " INVITE " + targetNick + " :" + channelName;
	sendResponse(targetUser->getFd(), inviteMsg);

	#ifdef DEBUG
	std::cout << "User " << user->getNickname()
			  << " invited " << targetNick
			  << " to channel " << channelName << std::endl;
	#endif
}
