/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 17:16:17 by spyun             #+#    #+#             */
/*   Updated: 2025/11/25 13:41:51 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Commands.hpp"
#include "Server.hpp"
#include "Message.hpp"

// ==================== Channel Helper ====================

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

// ==================== Main Execute Command ====================

void Commands::executeCommand(User* user, const std::string& command,
								const std::vector<std::string>& params,
								Server& server,
								const std::string& serverPassword)
{
	if (!Authentication::canExecuteCommand(user, command))
	{
		Authentication::sendAuthenticationError(user->getFd(), command);
		return;
	}

	switch ( getCmdType(command) )
	{
	case CMD_PASS:
		handlePASS(user, params, serverPassword);
		break;
	case CMD_NICK:
		handleNICK(user, params, server);
		break;
	case CMD_USER:
		handleUSER(user, params);
		break;
	case CMD_JOIN:
		handleJOIN(user, params, server);
		break;
	case CMD_PRIVMSG:
		handlePRIVMSG(user, params, server);
		break;
	case CMD_KICK:
		handleKICK(user, params, server);
		break;
	case CMD_PART:
		handlePART(user, params, server);
		break;
	case CMD_TOPIC:
		handleTOPIC(user, params, server);
		break;
	case CMD_INVITE:
		handleINVITE(user, params, server);
		break;
	default:
		ResponseHandler::sendNumericReply(user->getFd(), 421, command + " :Unknown command");
		break;
	}

	// if (command == "PASS")
	// 	handlePASS(user, params, serverPassword);
	// else if (command == "NICK")
	// 	handleNICK(user, params, server);
	// else if (command == "USER")
	// 	handleUSER(user, params);
	// else if (command == "JOIN")
	// 	handleJOIN(user, params, server);
	// else if (command == "PRIVMSG")
	// 	handlePRIVMSG(user, params, server);
	// else if (command == "KICK")
	// 	handleKICK(user, params, server);
	// else if (command == "PART")
	// 	handlePART(user, params, server);
	// else if (command == "TOPIC")
	// 	handleTOPIC(user, params, server);
	// else if (command == "INVITE")
	// 	handleINVITE(user, params, server);
	// else
	// 	ResponseHandler::sendNumericReply(user->getFd(), 421, command + " :Unknown command");
}

// ==================== PASS Command ====================

void Commands::handlePASS(User* user, const std::vector<std::string>& params,
						  const std::string& serverPassword)
{
	if (!Validation::validatePASS(user, params))
		return;

	std::string providedPassword = ValidationHelper::removeLeadingColon(params.front());
	if (!ValidationHelper::isValidPassword(providedPassword))
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_PASSWDMISMATCH, ":Password contains invalid characters");
		return;
	}
	if (!Authentication::validatePassword(providedPassword, serverPassword))
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_PASSWDMISMATCH,  ":Password incorrect");
		return;
	}

	user->setPasswordProvided(true);
	user->setAuthenticated(true);

	Authentication::checkRegistration(user);

	#ifdef DEBUG
	std::cout << "User fd " << user->getFd() << " provided correct password." << std::endl;
	#endif
}

// ==================== NICK Command ====================

void Commands::handleNICK(User* user, const std::vector<std::string>& params,  Server& server)
{
	std::string newNick {};

	if (!Validation::validateNICK(user, params, server, newNick))
		return;

	std::string oldNick = user->getNickname();
	user->setNickname(newNick);
	user->setHasNickname(true);

	if (Authentication::isRegistered(user))
	{
		std::string nickChangeMsg = user->getPrefix() + " NICK :" + newNick;
		ResponseHandler::sendResponse(user->getFd(), nickChangeMsg);
	}

	Authentication::checkRegistration(user);

	#ifdef DEBUG
	std::cout << "User fd " << user->getFd()
			  << " changed nickname from '" << oldNick << "' to '" << newNick << "'" << std::endl;
	#endif
}

// ==================== USER Command ====================

void Commands::handleUSER(User* user, const std::vector<std::string>& params)
{
	if (!Validation::validateUSER(user, params))
		return;

	std::vector<std::string>::const_iterator it = params.begin();
	std::string username = *it++;
	std::advance(it, 2);
	std::string realname = ValidationHelper::removeLeadingColon(*it);

	user->setUsername(username);
	user->setRealname(realname);
	user->setHostname("localhost");
	user->setHasUsername(true);

	Authentication::checkRegistration(user);

	#ifdef DEBUG
	std::cout << "User fd " << user->getFd()
			  << " set username to '" << username
			  << "' and realname to '" << realname << "'" << std::endl;
	#endif
}

// ==================== JOIN Command ====================

void Commands::handleJOIN(User* user, const std::vector<std::string>& params, Server& server)
{
	if (!Validation::validateJOIN(user, params))
		return;

	std::string channelList = ValidationHelper::removeLeadingColon(params.front());
	std::string keyList;
	if (params.size() > 1)
	{
		std::vector<std::string>::const_iterator it = params.begin();
		++it;
		keyList = ValidationHelper::removeLeadingColon(*it);
	}

	std::vector<std::string> channelVec;
	std::istringstream channelStream(channelList);
	std::string channelName;
	while (std::getline(channelStream, channelName, ','))
	{
		if (!channelName.empty())
			channelVec.push_back(channelName);
	}

	std::vector<std::string> keys;
	if (!keyList.empty())
	{
		std::istringstream keyStream(keyList);
		std::string key;
		while (std::getline(keyStream, key, ','))
		{
			keys.push_back(key);
		}
	}

	std::map<std::string, Channel*>& channels = server.getChannels();

	for (size_t i = 0; i < channelVec.size(); ++i)
	{
		std::string currentChannel = channelVec[i];
		std::string channelKey = (i < keys.size()) ? keys[i] : "";

		if (!ValidationHelper::isValidChannelName(currentChannel))
		{
			ResponseHandler::sendNumericReply(user->getFd(), ERR_NOSUCHCHANNEL, currentChannel + " :No such channel");
			continue;
		}

		Channel* channel = getOrCreateChannel(currentChannel, channels);

		if (!Validation::validateCanJoin(user, channel, channelKey))
			continue;

		channel->addMember(user);
		user->joinChannel(currentChannel);

		if (channel->isInvited(user->getFd()))
			channel->removeInvite(user->getFd());

		ResponseHandler::sendJoinMessages(user, channel);

		#ifdef DEBUG
		std::cout << "User " << user->getNickname()
				  << " joined channel " << currentChannel << std::endl;
		#endif
	}
}

// ==================== PRIVMSG Handler ====================

void Commands::handlePRIVMSG(User* user, const std::vector<std::string>& params, Server& server)
{
	if (!Validation::validatePRIVMSG(user, params))
		return;

	std::vector<std::string>::const_iterator it = params.begin();
	std::string target = *it++;
	std::string message = *it;

	if (!message.empty() && message[0] == ':')
		message = message.substr(1);

	// for (; it != params.end(); ++it)  //DOMINIQUE: I removed this because it duplicated messages by the bot. I think this was a leftover from when we were not using Takato's parser class?
	// 	message += " " + *it;

	if (target[0] == '#' || target[0] == '&')
	{
		Channel* channel = Validation::validateCanSendMsg(user, target, server);

		if (!channel)
			return;

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

		const std::map<int, User*>& users = server.getUsers();
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
			ResponseHandler::sendNumericReply(user->getFd(), ERR_NOSUCHNICK, target + " :No such nick/channel");
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

void Commands::handleKICK(User* user, const std::vector<std::string>& params, Server& server)
{
	std::string	targetNick {};
	std::string	channelName {};
	std::string	reason {};

	if (!Validation::validateKICK(user, params, targetNick, channelName, reason))
		return;

	Channel* channel = Validation::validateCanKick(user, channelName, server);
	if(!channel)
		return;

	User* targetUser = Validation::validateCanKickTarget(user, channel, targetNick, server);
	if(!targetUser)
		return;

	std::string kickMsg = user->getPrefix() + " KICK " + channelName + " " + targetNick + " :" + reason;
	const std::map<int, User*>& members = channel->getMembers();
	for (std::map<int, User*>::const_iterator it = members.begin(); it != members.end(); ++it)
		ResponseHandler::sendResponse(it->second->getFd(), kickMsg);

	channel->removeMember(targetUser->getFd());
	targetUser->leaveChannel(channelName);

	if (channel->isEmpty())
	{
		server.getChannels().erase(channelName);
		delete channel;

		#ifdef DEBUG
		std::cout << "Channel " << channelName << " deleted as it became empty." << std::endl;
		#endif
	}

	#ifdef DEBUG
	std::cout << "User " << targetNick << " was kicked from channel " << channelName << " by " << user->getNickname() << std::endl;
	#endif
}

// ==================== PART Handler ====================

void Commands::handlePART(User* user, const std::vector<std::string>& params, Server& server)
{
	if (!Validation::validatePART(user, params))
		return;

	std::string channelList = params.front();
	if (!channelList.empty() && channelList[0] == ':')
		channelList = channelList.substr(1);

	std::string reason;
	if (params.size() > 1)
	{
		std::vector<std::string>::const_iterator it = params.begin();
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

		Channel* channel = Validation::validateCanPart(user, currentChannel, server);
		if (!channel)
			continue;

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
			server.getChannels().erase(currentChannel);
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

void Commands::handleTOPIC(User* user, const std::vector<std::string>& params, Server& server)
{
	std::string	channelName;

	if (!Validation::validateTOPIC(user, params, channelName))
		return;

	Channel* channel = Validation::validateCanChangeTopic(user, channelName, server);
	if(!channel)
		return;

	if (params.size() == 1)
	{
		const std::string& currentTopic = channel->getTopic();

		if (currentTopic.empty())
		{
			ResponseHandler::sendNumericReply(user->getFd(), RPL_NOTOPIC,
											  channelName + " :No topic is set");
		}
		else
		{
			std::ostringstream topicMsg;
			topicMsg << ":ft_irc " << std::setw(3) << std::setfill('0') << RPL_TOPIC
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
		ResponseHandler::sendNumericReply(user->getFd(), ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
		return;
	}

	std::vector<std::string>::const_iterator it = params.begin();
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

void Commands::handleINVITE(User* user, const std::vector<std::string>& params, Server& server)
{
	std::string	targetNick;
	std::string	channelName;

	if (!Validation::validateINVITE(user, params, targetNick, channelName))
		return;
	Channel* channel = Validation::validateCanInvite(user, channelName, server);
	if(!channel)
		return;
	User* targetUser = Validation::validateCanInviteTarget(user, channel, channelName, targetNick, server);
	if(!targetUser)
		return;

	channel->addInvite(targetUser->getFd());
	std::ostringstream invitingMsg;
	invitingMsg << ":ft_irc " << std::setw(3) << std::setfill('0') << RPL_INVITING
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
