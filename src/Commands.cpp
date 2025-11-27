/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 17:16:17 by spyun             #+#    #+#             */
/*   Updated: 2025/11/27 14:26:17 by dlippelt         ###   ########.fr       */
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
	case CMD_PING:
		handlePING(user, params);
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
	case CMD_QUIT:
		handleQUIT(user, params, server);
		break;
	case CMD_MODE:
		handleMODE(user, params, server);
		break;
	default:
		ResponseHandler::sendNumericReply(user->getFd(), 421, command + " :Unknown command");
		break;
	}
}

// ==================== PASS Command ====================

void Commands::handlePASS(User* user, const std::vector<std::string>& params, const std::string& serverPassword)
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

// ==================== PING Command ====================

void Commands::handlePING(User* user, const std::vector<std::string>& params)
{
	std::string pong_str;

	if (!params.empty())
		pong_str = "PONG " + params[0] + "\r\n";
	else
		pong_str = "PONG :ft_irc\r\n";

	ssize_t sent = send(user->getFd(), pong_str.c_str(), pong_str.length(), 0);

	#ifdef DEBUG
	if (sent < 0)
		std::cout << "Failed to send PONG to fd " << user->getFd() << std::endl;
	else
		std::cout << "Sent PONG response to client fd " << user->getFd() << std::endl;
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

	if (target[0] == '#' || target[0] == '&')
	{
		Channel* channel = Validation::validateCanSendMsg(user, target, server);

		if (!channel)
			return;

		std::string privmsgToChannel = user->getPrefix() + " PRIVMSG " + target + " :" + message;

		const std::map<int, User*>& members = channel->getMembers();
		for (std::map<int, User*>::const_iterator it = members.begin(); it != members.end(); ++it)
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
			ResponseHandler::sendNumericReply(user->getFd(), RPL_NOTOPIC, channelName + " :No topic is set");
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

// ==================== QUIT Handler ====================

void Commands::handleQUIT(User* user, const std::vector<std::string>& params, Server& server)
{
	std::string quitMessage;

	if (!Validation::validateQUIT(user, params, quitMessage))
		return;

	std::string quitMsg = user->getPrefix() + " QUIT :Quit: " + quitMessage;

	const std::vector<std::string>& channels = user->getChannels();
	std::map<std::string, Channel*>& serverChannels = server.getChannels();

	for (size_t i = 0; i < channels.size(); ++i)
	{
		std::map<std::string, Channel*>::iterator it = serverChannels.find(channels[i]);
		if (it != serverChannels.end())
		{
			Channel* channel = it->second;

			const std::map<int, User*>& members = channel->getMembers();
			for (std::map<int, User*>::const_iterator memIt = members.begin();
				 memIt != members.end(); ++memIt)
			{
				if (memIt->first != user->getFd())
					ResponseHandler::sendResponse(memIt->first, quitMsg);
			}

			channel->removeMember(user->getFd());

			if (channel->isEmpty())
			{
				serverChannels.erase(it);
				delete channel;

				#ifdef DEBUG
				std::cout << "Channel " << channels[i] << " deleted as it became empty." << std::endl;
				#endif
			}
		}
	}

	ResponseHandler::sendResponse(user->getFd(), "ERROR :Closing connection");

	#ifdef DEBUG
	std::cout << "User " << user->getNickname() << " quit: " << quitMessage << std::endl;
	#endif

	// close(user->getFd());	// You can't do this here like this.
	// 						// The fd is still part of the list of fd's that gets polled and if it doesn't get removed from that list poll() will keep returning an error because it's trying to poll a closed fd
	// 						// Also unless I'm missing it the client also needs to be removed from the user list.
	// 						// So instead I made the removeClient() method of the Server class public and call that one here.
	// 						// It takes care of removing the user from the user list, removing the FD from the polling list, and closes the fd

	server.removeClient(user->getFd());
}

// [Takato]: added from here
// Edited by Dominique
void	Commands::handleMODE(User *user, const std::vector<std::string>& params, Server &server)
{
	// Parameters: <channel> *( ( "-" / "+" ) *<modes> *<modeparams> )

	std::string		channelName {};
	Channel* 		channel { Validation::validateMODE(user, params, server, channelName) };

	if (!channel)
		return;

	if (params.size() == 1)
	{
		// Dominique:
		// NOTE: In Irssi, typing just '/mode' triggers client-side "Irssi: not enough parameters" warning, but
		// Irssi still auto-appends the current channel and sends "MODE #currentchannel".
		// However, Irssi then rejects/doesn't display the (correctly formatted) server response.
		// Unless you can find a solution this seems to be an irssi quirk we should just accept.
		ResponseHandler::sendNumericReply(user->getFd(), RPL_CHANNELMODEIS, user->getNickname(), channelName + " " + channel->getModeString());
		return;
	}

	if (!Validation::validateCanChangeModes(user, channel, channelName))
		return;

	const std::string	&modes{ params[1] };
	const char 			sign{ modes.front() };

	if (!Validation::validateModes(user, modes))
		return;

	int	modeSettingIdxOffset {};

	for (int i{ 1 }; i < static_cast<int>(std::min(modes.size(), k_max_mode_num + 1)); ++i)
	{
		if (!Validation::validateModeCharacter(user, modes[i], k_mode_set_param + k_mode_set_toggle))
			return;

		if (k_mode_set_toggle.find(modes[i]) != std::string::npos)
			modeOperateToggle(modes[i], sign, params, server);
		else if (k_mode_set_param.find(modes[i]) != std::string::npos)
		{
			if (params.size() < MINIMUM_PARAMS_MODE + 1)
			{
				ResponseHandler::sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "MODE :Not enough parameters");
				return;
			}
			try
			{
				modeOperateParam(modes[i], sign, params, server, modeSettingIdxOffset);
				if (!(sign == '-' && modes[i] == 'l'))
					modeSettingIdxOffset++;
			}
			catch ( IrcNumericCodes error_code )
			{
				Validation::handleModeOperationError(user, channelName, error_code);
				if (!(sign == '-' && modes[i] == 'l'))
					modeSettingIdxOffset++;
				continue;
			}
		}
	}

	#ifdef DEBUG
	server.printModeStates();
	#endif
}

void	Commands::modeOperateToggle(char mode, char sign, const std::vector<std::string>& params, Server &server)
{
	switch (mode)
	{
	case 'i':
		modeOperateToggleInvite(sign, params, server);
		break ;
	case 't':
		modeOperateToggleTopic(sign, params, server);
		break ;
	default:
		break ;
	}
}

void	Commands::modeOperateParam(char mode, char sign, const std::vector<std::string>& params, Server &server, int idxOffset)
{
	switch (mode)
	{
	case 'k':
		modeOperateParamKey(sign, params, server, idxOffset);
		break ;
	case 'o':
		modeOperateParamPrivilege(sign, params, server, idxOffset);
		break;
	case 'l':
		modeOperateParamLimit(sign, params, server, idxOffset);
		break;
	default:
		break;
	}
}

void	Commands::modeOperateToggleInvite(char sign, const std::vector<std::string>& params, Server &server)
{
	const std::string	&channel_name { params[0] };
	Channel				*channel { server.getChannels().at(channel_name)};

	switch (sign)
	{
	case '+':
		channel->setInviteOnly(true);
		break ;
	case '-':
		channel->setInviteOnly(false);
		break ;
	default:
		break ;
	}
}

void	Commands::modeOperateToggleTopic(char sign, const std::vector<std::string>& params, Server &server)
{
	const std::string	&channel_name { params[0] };
	Channel				*channel { server.getChannels().at(channel_name)};

	switch (sign)
	{
	case '+':
		channel->setTopicRestricted(true);
		break;
	case '-':
		channel->setTopicRestricted(false);
		break;
	default:
		break;
	}
}

void	Commands::modeOperateParamKey(char sign, const std::vector<std::string>& params, Server &server, int idxOffset)
{
	const std::string	&channel_name { params[0] };
	const std::string	&param { params[MINIMUM_PARAMS_MODE + idxOffset] };
	Channel				*channel { server.getChannels().at(channel_name) };

	switch (sign)
	{
	case '+':
		if (channel->hasKey())
			throw ERR_KEYSET;
		channel->setKey(param);
		channel->setHasKey(true);
		break;
	case '-':
		channel->setHasKey(false);
		break;
	default:
		break;
	}
}

void	Commands::modeOperateParamPrivilege(char sign, const std::vector<std::string>& params, Server &server, int idxOffset)
{
	const std::string	&channel_name { params[0] };
	const std::string	&param { params[MINIMUM_PARAMS_MODE + idxOffset] };
	Channel				*channel { server.getChannels().at(channel_name) };
	User				*user {};

	for (auto it{ server.getUsers().begin() }; it != server.getUsers().end(); ++it)
	{
		if (it->second->getNickname() == param)
		{
			user = it->second;
			break ;
		}
	}
	if (!user || !user->isInChannel(channel_name))
		throw ERR_NOTONCHANNEL;
	switch (sign)
	{
	case '+':
		channel->addOperator(user->getFd());
		break ;
	case '-':
		channel->removeOperator(user->getFd());
		break ;
	default:
		break ;
	}
}

void	Commands::modeOperateParamLimit(char sign, const std::vector<std::string>& params, Server &server, int idxOffset)
{
	const std::string	&channel_name { params[0] };
	const std::string	&param { params[MINIMUM_PARAMS_MODE + idxOffset] };
	Channel				*channel { server.getChannels().at(channel_name) };

	switch (sign)
	{
	case '+':
		try
		{
			channel->setUserLimit(std::stoi(param));
		}
		catch(const std::exception& e)
		{
			throw ERR_NEEDMOREPARAMS;
		}
		channel->setHasUserLimit(true);
		break ;
	case '-':
		channel->setHasUserLimit(false);
		break ;
	default:
		break ;
	}
}

/* Channel Namespace
Channels names are strings (beginning with a '&', '#', '+' or '!'
   character) of length up to fifty (50) characters.
   Channel names are case insensitive.
   Apart from the the requirement that the first character being either
   '&', '#', '+' or '!' (hereafter called "channel prefix"). The only
   restriction on a channel name is that it SHALL NOT contain any spaces
   (' '), a control G (^G or ASCII 7), a comma (',' which is used as a
   list item separator by the protocol).  Also, a colon (':') is used as
   a delimiter for the channel mask.
 */

/* Channel Modes
Mandatory implementation: i, t, k, o, l

Note that there is a maximum limit of three (3) changes per command for modes that take a parameter.
	The various modes available for channels are as follows:
	O - give "channel creator" status;
	o - give/take channel operator privilege;
	v - give/take the voice privilege;
	a - toggle the anonymous channel flag;
	i - toggle the invite-only channel flag;
	m - toggle the moderated channel;
	n - toggle the no messages to channel from clients on the
	    outside;
	q - toggle the quiet channel flag;
	p - toggle the private channel flag;
	s - toggle the secret channel flag;
	r - toggle the server reop channel flag;
	t - toggle the topic settable by channel operator only flag;
	k - set/remove the channel key (password);
	l - set/remove the user limit to channel;
	b - set/remove ban mask to keep users out;
	e - set/remove an exception mask to override a ban mask;
	I - set/remove an invitation mask to automatically override
	    the invite-only flag;
	https://datatracker.ietf.org/doc/html/rfc2811

	Channel modes can be manipulated by the channel members.
	The modes affect the way servers manage the channels.
	Channels with '+' as prefix do not support channel modes.
	This means that all the modes are unset, with the exception of the 't' channel flag which is set.

	In order for the channel members to keep some control over a channel,
    and some kind of sanity, some channel members are privileged.  Only
	these members are allowed to perform the following actions on the channel:
        INVITE  - Invite a client to an invite-only channel (mode +i)
        KICK    - Eject a client from the channel
        MODE    - Change the channel's mode, as well as
                  members' privileges
        PRIVMSG - Sending messages to the channel (mode +n, +m, +v)
        TOPIC   - Change the channel topic in a mode +t channel

	Since channels starting with the character '+' as prefix do not
    support channel modes, no member can therefore have the status of channel operator.

 */

// [Takato]: added to here
