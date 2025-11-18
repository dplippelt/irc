/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Commands.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/30 17:16:17 by spyun         #+#    #+#                 */
/*   Updated: 2025/11/18 09:50:16 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Commands.hpp"

// ==================== Helper Functions ====================

bool Commands::isValidNickname(const std::string& nick)
{
	// IRC nickname rules:
	// - Must start with a letter
	// - Can contain letters, numbers, and special chars: - [ ] \ ` ^ { }
	// - Length between 1 and 9 characters (we'll use 1-20 for flexibility)

	if (nick.empty() || nick.length() > 20)
		return false;
	if (!std::isalpha(static_cast<unsigned char>(nick[0])))
		return false;
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

bool Commands::isNicknameInUse(const std::string& nick, const std::map<int, User*>& users)
{
	for (std::map<int, User*>::const_iterator it = users.begin();
		 it != users.end(); ++it)
	{
		if (it->second->getNickname() == nick)
			return true;
	}
	return false;
}

bool Commands::isValidChannelName(const std::string& channelName)
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

Channel* Commands::getOrCreateChannel(const std::string& channelName, std::map<std::string, Channel*>& channels)
{
	std::map<std::string, Channel*>::iterator it = channels.find(channelName);
	if (it != channels.end())
		return it->second;

	Channel* newChannel = new Channel(channelName);
	channels[channelName] = newChannel;

	#ifdef DEBUG
	std::cout << "Created new channel: " << channelName << std::endl;
	#endif

	return newChannel;
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
		ResponseHandler::sendWelcome(user);

		#ifdef DEBUG
		std::cout << "User " << user->getNickname() << " (fd " << user->getFd() << ") is now registered!" << std::endl;
        #endif
	}
}

bool Commands::canExecuteCommand(User* user, const std::string& command)
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

// ==================== Main Execute Command ====================

void Commands::executeCommand(User* user, const std::string& command,
								const std::vector<std::string>& params,
								std::map<int, User*>& users,
								std::map<std::string, Channel*>& channels,
								const std::string& serverPassword)
{
	if (!canExecuteCommand(user, command))
	{
		ResponseHandler::sendAuthenticationError(user->getFd(), command);
		return;
	}

	std::list<std::string> paramList(params.begin(), params.end());

	if (command == "PASS")
		handlePASS(user, paramList, serverPassword);
	else if (command == "NICK")
		handleNICK(user, paramList, users);
	else if (command == "USER")
		handleUSER(user, paramList);
	else if (command == "JOIN")
		handleJOIN(user, paramList, channels);
	else if (command == "PRIVMSG")
		handlePRIVMSG(user, paramList, users, channels);
	else if (command == "KICK")
		handleKICK(user, paramList, users, channels);
	else if (command == "PART")
		handlePART(user, paramList, channels);
	else if (command == "TOPIC")
		handleTOPIC(user, paramList, channels);
	else if (command == "INVITE")
		handleINVITE(user, paramList, users, channels);
	else
	{
		ResponseHandler::sendNumericReply(user->getFd(), 421, command + " :Unknown command");
	}
}

// ==================== Command Handlers ====================

void Commands::handlePASS(User* user, const std::list<std::string>& params, const std::string& serverPassword)
{
	if (user->isRegistered())
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_ALREADYREGISTRED,
										  ":You may not reregister");
		return;
	}
	if (user->hasProvidedPassword())
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_ALREADYREGISTRED,
										  ":You have already provided a password");
		return;
	}
	if (params.empty())
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NEEDMOREPARAMS,
										  "PASS :Not enough parameters");
		return;
	}

	std::string providedPassword = params.front();

	if (!providedPassword.empty() && providedPassword[0] == ':')
		providedPassword = providedPassword.substr(1);
	if (providedPassword != serverPassword)
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_PASSWDMISMATCH,
										  ":Password incorrect");
		return;
	}

	user->setPasswordProvided(true);
	user->setAuthenticated(true);

	checkRegistration(user);

	#ifdef DEBUG
	std::cout << "User fd " << user->getFd() << " provided correct password." << std::endl;
	#endif
}

void Commands::handleNICK(User* user, const std::list<std::string>& params, const std::map<int, User*>& users)
{
	if (params.empty())
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NONICKNAMEGIVEN,
										  ":No nickname given");
		return;
	}

	std::string newNick = params.front();
	if (!newNick.empty() && newNick[0] == ':')
		newNick = newNick.substr(1);
	if (!isValidNickname(newNick))
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_ERRONEUSNICKNAME,
										  newNick + " :Erroneous nickname");
		return;
	}
	if (isNicknameInUse(newNick, users))
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NICKNAMEINUSE,
										  newNick + " :Nickname is already in use");
		return;
	}

	std::string oldNick = user->getNickname();
	user->setNickname(newNick);
	user->setHasNickname(true);

	if (user->isRegistered())
	{
		std::string nickChangeMsg = user->getPrefix() + " NICK :" + newNick;
		ResponseHandler::sendResponse(user->getFd(), nickChangeMsg);
	}

	checkRegistration(user);

	#ifdef DEBUG
	std::cout << "User fd" << user->getFd() << " set nickname to " << newNick << std::endl;
	#endif
}

void Commands::handleUSER(User* user, const std::list<std::string>& params)
{
	if (user->isRegistered())
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_ALREADYREGISTRED,
										  ":You may not reregister");
		return;
	}
	if (user->hasUsername())
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_ALREADYREGISTRED,
										  ":You have already set a username");
		return;
	}
	if (params.size() < 4)
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NEEDMOREPARAMS,
										  "USER :Not enough parameters");
		return;
	}

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
// ==================== JOIN Handler ====================

void Commands::handleJOIN(User* user, const std::list<std::string>& params, std::map<std::string, Channel*>& channels)
{
	if (!user->isRegistered())
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NOTREGISTERED,
										  ":You have not registered");
		return;
	}
	if (params.empty())
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NEEDMOREPARAMS,
										  "JOIN :Not enough parameters");
		return;
	}

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

	std::vector<std::string> channelVec;
	std::vector<std::string> keys;

	std::istringstream channelStream(channelList);
	std::string channe;
	while (std::getline(channelStream, channe, ','))
	{
		if (!channe.empty())
			channelVec.push_back(channe);
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

	for (size_t i = 0; i < channelVec.size(); ++i)
	{
		std::string channelName = channelVec[i];
		std::string channelKey = (i < keys.size()) ? keys[i] : "";

		if (!isValidChannelName(channelName))
		{
			ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NOSUCHCHANNEL,
											  channelName + " :No such channel");
			continue;
		}

		Channel* channel = getOrCreateChannel(channelName, channels);

		if (channel->isMember(user->getFd()))
			continue;

		if (channel->isInviteOnly() && !channel->isInvited(user->getFd()))
		{
			ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_INVITEONLYCHAN,
											  channelName + " :Cannot join channel (+i)");
			continue;
		}
		if (channel->hasKey())
		{
			if (channelKey.empty() || channelKey != channel->getKey())
			{
				ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_BADCHANNELKEY,
												  channelName + " :Cannot join channel (+k)");
				continue;
			}
		}
		if (channel->hasUserLimit())
		{
			if (static_cast<int>(channel->getMemberCount()) >= channel->getUserLimit())
			{
				ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_CHANNELISFULL,
												  channelName + " :Cannot join channel (+l)");
				continue;
			}
		}

		channel->addMember(user);
		user->joinChannel(channelName);

		if (channel->isInvited(user->getFd()))
			channel->removeInvite(user->getFd());

		ResponseHandler::sendJoinMessages(user, channel);

		#ifdef DEBUG
		std::cout << "User " << user->getNickname()
				  << " joined channel " << channelName << std::endl;
		#endif
	}
}

// ==================== PRIVMSG Handler ====================

void Commands::handlePRIVMSG(User* user, const std::list<std::string>& params, const std::map<int, User*>& users, std::map<std::string, Channel*>& channels)
{
	if (!user->isRegistered())
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NOTREGISTERED,
										  ":You have not registered");
		return;
	}
	if (params.empty())
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NORECIPIENT,
										  "PRIVMSG :No recipient given");
		return;
	}
	if (params.size() < 2)
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NEEDMOREPARAMS,
										  "PRIVMSG :Not enough parameters");
		return;
	}

	std::list<std::string>::const_iterator it = params.begin();
	std::string target = *it++;
	std::string message = *it;

	if (!message.empty() && message[0] == ':')
		message = message.substr(1);

	for (; it != params.end(); ++it)
		message += " " + *it;

	if (target[0] == '#' || target[0] == '&')
	{
		std::map<std::string, Channel*>::iterator it = channels.find(target);
		if (it == channels.end())
		{
			ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NOSUCHCHANNEL,
											  target + " :No such channel");
			return;
		}

		Channel* channel = it->second;

		if (!channel->isMember(user->getFd()))
		{
			ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_CANNOTSENDTOCHAN,
											  target + " :Cannot send to channel");
			return;
		}

		std::string privmsgToChannel = user->getPrefix() + " PRIVMSG " + target + " :" + message;

		const std::map<int, User*>& members = channel->getMembers();
		for (std::map<int, User*>::const_iterator it = members.begin();
			 it != members.end(); ++it)
		{
			if (it->first != user->getFd())
				ResponseHandler::sendResponse(it->second->getFd(), privmsgToChannel);
		}

		#ifdef DEBUG
		std::cout << "User " << user->getNickname()
				  << " sent message to channel " << target << std::endl;
		#endif
	}
	else
	{
		User* targetUser = nullptr;

		for (std::map<int, User*>::const_iterator it = users.begin();
			 it != users.end(); ++it)
		{
			if (it->second->getNickname() == target)
			{
				targetUser = it->second;
				break;
			}
		}

		if (targetUser == nullptr)
		{
			ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NOSUCHNICK,
											  target + " :No such nick/channel");
			return;
		}

		std::string privmsgToUser = user->getPrefix() + " PRIVMSG " + target + " :" + message;
		ResponseHandler::sendResponse(targetUser->getFd(), privmsgToUser);

		#ifdef DEBUG
		std::cout << "User " << user->getNickname()
				  << " sent DM to " << target << ": " << message << std::endl;
		#endif
	}
}
// ==================== KICK Handler ====================

void Commands::handleKICK(User* user, const std::list<std::string>& params, const std::map<int, User*>& users, std::map<std::string, Channel*>& channels)
{
	if (!user->isRegistered())
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NOTREGISTERED,
										  ":You have not registered");
		return;
	}
	if (params.size() < 2)
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NEEDMOREPARAMS,
										  "KICK :Not enough parameters");
		return;
	}

	std::list<std::string>::const_iterator it = params.begin();
	std::string channelName = *it++;
	std::string targetNick = *it++;

	std::string reason = "Kicked by operator";
	if (it != params.end())
	{
		reason = *it;
		if (!reason.empty() && reason[0] == ':')
			reason = reason.substr(1);
	}

	std::map<std::string, Channel*>::iterator chanIt = channels.find(channelName);
	if (chanIt == channels.end())
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NOSUCHCHANNEL,
										  channelName + " :No such channel");
		return;
	}

	Channel* channel = chanIt->second;

	if (!channel->isMember(user->getFd()))
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NOTONCHANNEL,
										  channelName + " :You're not on that channel");
		return;
	}
	if (!channel->isOperator(user->getFd()))
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_CHANOPRIVSNEEDED,
										  channelName + " :You're not channel operator");
		return;
	}

	User* targetUser = nullptr;
	for (std::map<int, User*>::const_iterator it = users.begin(); it != users.end(); ++it)
	{
		if (it->second->getNickname() == targetNick)
		{
			targetUser = it->second;
			break;
		}
	}

	if (!channel->isMember(targetUser->getFd()))
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_USERNOTINCHANNEL,
										  targetNick + " " + channelName + " :They aren't on that channel");
		return;
	}

	std::string kickMsg = user->getPrefix() + " KICK " + channelName + " " + targetNick + " :" + reason;
	const std::map<int, User*>& members = channel->getMembers();
	for (std::map<int, User*>::const_iterator it = members.begin(); it != members.end(); ++it)
		ResponseHandler::sendResponse(it->second->getFd(), kickMsg);

	channel->removeMember(targetUser->getFd());
	targetUser->leaveChannel(channelName);

	if (channel->isEmpty())
	{
		channels.erase(channelName);
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

// ==================== PART Handler ====================

void Commands::handlePART(User* user, const std::list<std::string>& params, std::map<std::string, Channel*>& channels)
{
	if (!user->isRegistered())
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NOTREGISTERED,
										  ":You have not registered");
		return;
	}
	if (params.empty())
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NEEDMOREPARAMS,
										  "PART :Not enough parameters");
		return;
	}

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

	std::vector<std::string> channelVec;
	std::istringstream channelStream(channelList);
	std::string channelName;
	while (std::getline(channelStream, channelName, ','))
	{
		if (!channelName.empty())
			channelVec.push_back(channelName);
	}

	for (size_t i = 0; i < channelVec.size(); ++i)
	{
		std::string currentChannel = channelVec[i];

		std::map<std::string, Channel*>::iterator chanIt = channels.find(currentChannel);
		if (chanIt == channels.end())
		{
			ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NOSUCHCHANNEL,
							currentChannel + " :No such channel");
			continue;
		}

		Channel* channel = chanIt->second;

		if (!channel->isMember(user->getFd()))
		{
			ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NOTONCHANNEL,
											  currentChannel + " :You're not on that channel");
			continue;
		}

		std::string partMsg = user->getPrefix() + " PART " + currentChannel;
		if (!reason.empty())
			partMsg += " :" + reason;

		const std::map<int, User*>& members = channel->getMembers();
		for (std::map<int, User*>::const_iterator it = members.begin();
			it != members.end(); ++it)
		{
			ResponseHandler::sendResponse(it->second->getFd(), partMsg);
		}

		channel->removeMember(user->getFd());
		user->leaveChannel(currentChannel);

		if (channel->isEmpty())
		{
			channels.erase(currentChannel);
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

// ==================== TOPIC Handler ====================

void Commands::handleTOPIC(User* user, const std::list<std::string>& params, std::map<std::string, Channel*>& channels)
{
	if (!user->isRegistered())
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NOTREGISTERED,
										  ":You have not registered");
		return;
	}
	if (params.empty())
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NEEDMOREPARAMS,
										  "TOPIC :Not enough parameters");
		return;
	}
	std::string channelName = params.front();
	if (!channelName.empty() && channelName[0] == ':')
		channelName = channelName.substr(1);

	std::map<std::string, Channel*>::iterator chanIt = channels.find(channelName);
	if (chanIt == channels.end())
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NOSUCHCHANNEL,
										  channelName + " :No such channel");
		return;
	}

	Channel* channel = chanIt->second;

	if (!channel->isMember(user->getFd()))
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NOTONCHANNEL,
										  channelName + " :You're not on that channel");
		return;
	}

	if (params.size() == 1)
	{
		const std::string& currentTopic = channel->getTopic();

		if (currentTopic.empty())
		{
			ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::RPL_NOTOPIC,
											  channelName + " :No topic is set");
		}
		else
		{
			std::ostringstream topicMsg;
			topicMsg << ":ft_irc " << std::setw(3) << std::setfill('0') << ResponseHandler::RPL_TOPIC
					 << " " << user->getNickname() << " "
					 << channelName << " :" << currentTopic;
			ResponseHandler::sendResponse(user->getFd(), topicMsg.str());
		}

		#ifdef DEBUG
		std::cout << "User " << user->getNickname() << " viewed topic of " << channelName << std::endl;
		#endif

		return;
	}

	if (channel->isTopicRestricted() && !channel->isOperator(user->getFd()))
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_CHANOPRIVSNEEDED,
										  channelName + " :You're not channel operator");
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
		ResponseHandler::sendResponse(memIt->second->getFd(), topicChangeMsg);
	}

	#ifdef DEBUG
	std::cout << "User " << user->getNickname()
			  << " changed topic of " << channelName
			  << " to: " << newTopic << std::endl;
	#endif
}

// ==================== INVITE Handler ====================

void Commands::handleINVITE(User* user, const std::list<std::string>& params, const std::map<int, User*>& users, std::map<std::string, Channel*>& channels)
{
	if (!user->isRegistered())
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NOTREGISTERED,
										  ":You have not registered");
		return;
	}
	if (params.size() < 2)
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NEEDMOREPARAMS,
										  "INVITE :Not enough parameters");
		return;
	}

	std::list<std::string>::const_iterator it = params.begin();
	std::string targetNick = *it++;
	std::string channelName = *it;

	if (!channelName.empty() && channelName[0] == ':')
		channelName = channelName.substr(1);
	if (!targetNick.empty() && targetNick[0] == ':')
		targetNick = targetNick.substr(1);

	std::map<std::string, Channel*>::iterator chanIt = channels.find(channelName);
	if (chanIt == channels.end())
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NOSUCHCHANNEL,
										  channelName + " :No such channel");
		return;
	}

	Channel* channel = chanIt->second;
	if (!channel->isMember(user->getFd()))
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NOTONCHANNEL,
										  channelName + " :You're not on that channel");
		return;
	}
	if (channel->isInviteOnly() && !channel->isOperator(user->getFd()))
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_CHANOPRIVSNEEDED,
										  channelName + " :You're not channel operator");
		return;
	}

	User* targetUser = nullptr;
	for (std::map<int, User*>::const_iterator it = users.begin(); it != users.end(); ++it)
	{
		if (it->second->getNickname() == targetNick)
		{
			targetUser = it->second;
			break;
		}
	}
	if (targetUser == nullptr)
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_NOSUCHNICK,
										  targetNick + " :No such nick/channel");
		return;
	}
	if (channel->isMember(targetUser->getFd()))
	{
		ResponseHandler::sendNumericReply(user->getFd(), ResponseHandler::ERR_USERONCHANNEL,
										  targetNick + " " + channelName + " :is already on channel");
		return;
	}

	channel->addInvite(targetUser->getFd());
	std::ostringstream invitingMsg;
	invitingMsg << ":ft_irc " << std::setw(3) << std::setfill('0') << ResponseHandler::RPL_INVITING
				<< " " << user->getNickname() << " "
				<< targetNick << " " << channelName;
	ResponseHandler::sendResponse(user->getFd(), invitingMsg.str());

	std::string inviteMsg = user->getPrefix() + " INVITE " + targetNick + " :" + channelName;
	ResponseHandler::sendResponse(targetUser->getFd(), inviteMsg);

	#ifdef DEBUG
	std::cout << "User " << user->getNickname()
			  << " invited " << targetNick
			  << " to channel " << channelName << std::endl;
	#endif
}
