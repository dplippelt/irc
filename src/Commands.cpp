/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 17:16:17 by spyun             #+#    #+#             */
/*   Updated: 2025/12/17 13:19:05 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Commands.hpp"
#include "Server.hpp"
#include "Message.hpp"
#include "CTCPHandler.hpp"

// =================== Constructor & Destructor ==================

Command::Command(Server& server, User* user, Message& msg)
	: m_server {server}
	, m_responseHandler {server}
	, m_user {user}
	, m_command { msg.getCommandName() }
	, m_params { msg.getParamsList() }
{
}

Command::~Command() = default;

// ==================== Channel Helper ====================

Channel* Command::getOrCreateChannel(const std::string& channelName, std::map<std::string, Channel*>& channels)
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

void Command::executeCommand()
{
	if (!Authentication::canExecuteCommand(m_user, m_command))
	{
		Authentication::sendAuthenticationError(m_user->getFd(), m_command, m_responseHandler);
		return;
	}

	switch ( getCmdType() )
	{
	case CMD_PASS:
		handlePASS();
		break;
	case CMD_NICK:
		handleNICK();
		break;
	case CMD_USER:
		handleUSER();
		break;
	case CMD_PING:
		handlePING();
		break;
	case CMD_JOIN:
		handleJOIN();
		break;
	case CMD_PRIVMSG:
		handlePRIVMSG();
		break;
	case CMD_KICK:
		handleKICK();
		break;
	case CMD_PART:
		handlePART();
		break;
	case CMD_TOPIC:
		handleTOPIC();
		break;
	case CMD_INVITE:
		handleINVITE();
		break;
	case CMD_QUIT:
		handleQUIT();
		break;
	case CMD_WHOIS:
		handleWHOIS();
		break;
	case CMD_MODE:
		handleMODE();
		break;
	default:
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_UNKNOWNCMD, m_user->getNickname(),  m_command + " :Unknown command");
		break;
	}
}

// ==================== PASS Command ====================

void Command::handlePASS()
{
	if (!Validation::validatePASS(m_user, m_params, m_responseHandler))
		return;

	std::string providedPassword = ValidationHelper::removeLeadingColon(m_params.front());
	if (!ValidationHelper::isValidPassword(providedPassword))
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_PASSWDMISMATCH, m_user->getNickname(), ":Password contains invalid characters");
		return;
	}
	if (!Authentication::validatePassword(providedPassword, m_server.getPassword()))
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_PASSWDMISMATCH, m_user->getNickname(),  ":Password incorrect");
		return;
	}

	m_user->setPasswordProvided(true);
	m_user->setAuthenticated(true);

	Authentication::checkRegistration(m_user, m_responseHandler);

	#ifdef DEBUG
	std::cout << "User fd " << m_user->getFd() << " provided correct password." << std::endl;
	#endif
}

// ==================== NICK Command ====================

void Command::handleNICK()
{
	std::string newNick {};

	if (!Validation::validateNICK(m_user, m_params, m_server, newNick, m_responseHandler))
		return;

	std::string oldNick = m_user->getNickname();
	m_user->setNickname(newNick);
	m_user->setHasNickname(true);

	if (Authentication::isRegistered(m_user))
	{
		std::string nickChangeMsg = m_user->getPrefix() + " NICK :" + newNick;
		m_responseHandler.sendResponse(m_user->getFd(), nickChangeMsg);
	}

	Authentication::checkRegistration(m_user, m_responseHandler);

	#ifdef DEBUG
	std::cout << "User fd " << m_user->getFd()
			  << " changed nickname from '" << oldNick << "' to '" << newNick << "'" << std::endl;
	#endif
}

// ==================== USER Command ====================

void Command::handleUSER()
{
	if (!Validation::validateUSER(m_user, m_params, m_responseHandler))
		return;

	std::vector<std::string>::const_iterator it = m_params.begin();
	std::string username = *it++;
	std::advance(it, 2);
	std::string realname = ValidationHelper::removeLeadingColon(*it);

	m_user->setUsername(username);
	m_user->setRealname(realname);
	m_user->setHostname("localhost");
	m_user->setHasUsername(true);

	Authentication::checkRegistration(m_user, m_responseHandler);

	#ifdef DEBUG
	std::cout << "User fd " << m_user->getFd()
			  << " set username to '" << username
			  << "' and realname to '" << realname << "'" << std::endl;
	#endif
}

// ==================== PING Command ====================

void Command::handlePING()
{
	std::string pong_str;

	if (!m_params.empty())
		pong_str = "PONG " + m_params[0] + "\r\n";
	else
		pong_str = "PONG :ft_irc\r\n";

	ssize_t sent = send(m_user->getFd(), pong_str.c_str(), pong_str.length(), 0);

	#ifdef DEBUG
	if (sent < 0)
		std::cout << "Failed to send PONG to fd " << m_user->getFd() << std::endl;
	else
		std::cout << "Sent PONG response to client fd " << m_user->getFd() << std::endl;
    #endif
}

// ==================== JOIN Command ====================

void Command::handleJOIN()
{
	if (!Validation::validateJOIN(m_user, m_params, m_responseHandler))
		return;

	std::string channelList = ValidationHelper::removeLeadingColon(m_params.front());
	std::string keyList;
	if (m_params.size() > 1)
	{
		std::vector<std::string>::const_iterator it = m_params.begin();
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

	std::map<std::string, Channel*>& channels = m_server.getChannels();

	for (size_t i = 0; i < channelVec.size(); ++i)
	{
		std::string currentChannel = channelVec[i];
		std::string channelKey = (i < keys.size()) ? keys[i] : "";

		if (!ValidationHelper::isValidChannelName(currentChannel))
		{
			m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOSUCHCHANNEL, m_user->getNickname(), currentChannel + " :No such channel");
			continue;
		}

		Channel* channel = getOrCreateChannel(currentChannel, channels);

		if (!Validation::validateCanJoin(m_user, channel, channelKey, m_responseHandler))
			continue;

		channel->addMember(m_user);
		m_user->joinChannel(currentChannel);

		if (channel->isInvited(m_user->getFd()))
			channel->removeInvite(m_user->getFd());

		m_responseHandler.sendJoinMessages(m_user, channel);

		#ifdef DEBUG
		std::cout << "User " << m_user->getNickname()
				  << " joined channel " << currentChannel << std::endl;
		#endif
	}
}

// ==================== PRIVMSG Handler ====================

void Command::handlePRIVMSG()
{
	if (!Validation::validatePRIVMSG(m_user, m_params, m_responseHandler))
		return;

	std::vector<std::string>::const_iterator it = m_params.begin();
	std::string target = *it++;
	std::string message = *it;

	if (!message.empty() && message[0] == ':')
		message = message.substr(1) + "\r\n";

	if (CTCPHandler::isCTCPMessage(message))
	{
		std::string ctcpCommand = CTCPHandler::extractCTCPCommand(message);
		if (CTCPHandler::isDCCCommand(ctcpCommand))
		{
			std::string filename;
			unsigned long ip;
			unsigned int port;
			unsigned long filesize;

			if (CTCPHandler::parseDCCSend(ctcpCommand, filename, ip, port, filesize))
            {
				#ifdef DEBUG
				std::string ipStr = CTCPHandler::ipIntToString(ip);
				std::cout << "DCC SEND detected: " << m_user->getNickname()
							<< " → " << target << std::endl;
				std::cout << "  File: " << filename << std::endl;
				std::cout << "  IP: " << ipStr << " (" << ip << ")" << std::endl;
				std::cout << "  Port: " << port << std::endl;
				std::cout << "  Size: " << filesize << " bytes" << std::endl;
				#endif
			}
		}
	}

	if (target[0] == '#' || target[0] == '&')
	{
		Channel* channel = Validation::validateCanSendMsg(m_user, target, m_server, m_responseHandler);

		if (!channel)
			return;

		std::string privmsgToChannel = m_user->getPrefix() + " PRIVMSG " + target + " :" + message;
		channel->broadcast(privmsgToChannel, m_server, m_user->getFd());

		#ifdef DEBUG
		std::cout << "User " << m_user->getNickname()
				  << " sent message to channel " << target << std::endl;
		#endif
	}
	else
	{
		User* targetUser = nullptr;

		const std::map<int, User*>& users = m_server.getUsers();
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
			m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOSUCHNICK, m_user->getNickname(), target + " :No such nick/channel");
			return;
		}

		std::string privmsgToUser = m_user->getPrefix() + " PRIVMSG " + target + " :" + message;
		m_server.sendToClient(targetUser->getFd(), privmsgToUser);

		#ifdef DEBUG
		std::cout << "User " << m_user->getNickname()
				  << " sent DM to " << target << ": " << message << std::endl;
		#endif
	}
}

// ==================== KICK Handler ====================

void Command::handleKICK()
{
	std::string	targetNick {};
	std::string	channelName {};
	std::string	reason {};

	if (!Validation::validateKICK(m_user, m_params, targetNick, channelName, reason, m_responseHandler))
		return;

	Channel* channel = Validation::validateCanKick(m_user, channelName, m_server, m_responseHandler);
	if(!channel)
		return;

	User* targetUser = Validation::validateCanKickTarget(m_user, channel, targetNick, m_server, m_responseHandler);
	if(!targetUser)
		return;

	std::string kickMsg = m_user->getPrefix() + " KICK " + channelName + " " + targetNick + " :" + reason;
	const std::map<int, User*>& members = channel->getMembers();
	for (std::map<int, User*>::const_iterator it = members.begin(); it != members.end(); ++it)
		m_responseHandler.sendResponse(it->second->getFd(), kickMsg);

	channel->removeMember(targetUser->getFd());
	targetUser->leaveChannel(channelName);

	if (channel->isEmpty())
	{
		m_server.getChannels().erase(channelName);
		delete channel;

		#ifdef DEBUG
		std::cout << "Channel " << channelName << " deleted as it became empty." << std::endl;
		#endif
	}

	#ifdef DEBUG
	std::cout << "User " << targetNick << " was kicked from channel " << channelName << " by " << m_user->getNickname() << std::endl;
	#endif
}

// ==================== PART Handler ====================

void Command::handlePART()
{
	if (!Validation::validatePART(m_user, m_params, m_responseHandler))
		return;

	std::string channelList = m_params.front();
	if (!channelList.empty() && channelList[0] == ':')
		channelList = channelList.substr(1);

	std::string reason;
	if (m_params.size() > 1)
	{
		std::vector<std::string>::const_iterator it = m_params.begin();
		++it;
		reason = *it;
		if (!reason.empty() && reason[0] == ':')
			reason = reason.substr(1);
		for (++it; it != m_params.end(); ++it)
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

		Channel* channel = Validation::validateCanPart(m_user, currentChannel, m_server, m_responseHandler);
		if (!channel)
			continue;

		std::string partMsg = m_user->getPrefix() + " PART " + currentChannel;
		if (!reason.empty())
			partMsg += " :" + reason;

		const std::map<int, User*>& members = channel->getMembers();
		for (std::map<int, User*>::const_iterator it = members.begin();
			it != members.end(); ++it)
		{
			m_responseHandler.sendResponse(it->second->getFd(), partMsg);
		}

		channel->removeMember(m_user->getFd());
		m_user->leaveChannel(currentChannel);

		if (channel->isEmpty())
		{
			m_server.getChannels().erase(currentChannel);
			delete channel;

			#ifdef DEBUG
			std::cout << "Channel " << currentChannel << " deleted as it became empty." << std::endl;
			#endif
		}

		#ifdef DEBUG
		std::cout << "User " << m_user->getNickname() << " left channel " << currentChannel;
		if (!reason.empty())
			std::cout << " (reason: " << reason << ")";
		std::cout << std::endl;
		#endif
	}
}

// ==================== TOPIC Handler ====================

void Command::handleTOPIC()
{
	std::string	channelName;

	if (!Validation::validateTOPIC(m_user, m_params, channelName, m_responseHandler))
		return;

	Channel* channel = Validation::validateCanChangeTopic(m_user, channelName, m_server, m_responseHandler);
	if(!channel)
		return;

	if (m_params.size() == 1)
	{
		const std::string& currentTopic = channel->getTopic();

		if (currentTopic.empty())
		{
			m_responseHandler.sendNumericReply(m_user->getFd(), RPL_NOTOPIC, m_user->getNickname(), channelName + " :No topic is set");
		}
		else
		{
			std::ostringstream topicMsg;
			topicMsg << ":ft_irc " << std::setw(3) << std::setfill('0') << RPL_TOPIC
					 << " " << m_user->getNickname() << " "
					 << channelName << " :" << currentTopic;
			m_responseHandler.sendResponse(m_user->getFd(), topicMsg.str());
		}

		#ifdef DEBUG
		std::cout << "User " << m_user->getNickname() << " viewed topic of " << channelName << std::endl;
		#endif

		return;
	}

	if (channel->isTopicRestricted() && !channel->isOperator(m_user->getFd()))
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_CHANOPRIVSNEEDED, m_user->getNickname(), channelName + " :You're not channel operator");
		return;
	}

	std::vector<std::string>::const_iterator it = m_params.begin();
	++it;
	std::string newTopic = *it;

	if (!newTopic.empty() && newTopic[0] == ':')
		newTopic = newTopic.substr(1);

	for (++it; it != m_params.end(); ++it)
		newTopic += " " + *it;

	channel->setTopic(newTopic);
	std::string topicChangeMsg = m_user->getPrefix() + " TOPIC " + channelName + " :" + newTopic;

	const std::map<int, User*>& members = channel->getMembers();
	for (std::map<int, User*>::const_iterator memIt = members.begin();
		 memIt != members.end(); ++memIt)
	{
		m_responseHandler.sendResponse(memIt->second->getFd(), topicChangeMsg);
	}

	#ifdef DEBUG
	std::cout << "User " << m_user->getNickname()
			  << " changed topic of " << channelName
			  << " to: " << newTopic << std::endl;
	#endif
}

// ==================== INVITE Handler ====================

void Command::handleINVITE()
{
	std::string	targetNick;
	std::string	channelName;

	if (!Validation::validateINVITE(m_user, m_params, targetNick, channelName, m_responseHandler))
		return;
	Channel* channel = Validation::validateCanInvite(m_user, channelName, m_server, m_responseHandler);
	if(!channel)
		return;
	User* targetUser = Validation::validateCanInviteTarget(m_user, channel, channelName, targetNick, m_server, m_responseHandler);
	if(!targetUser)
		return;

	channel->addInvite(targetUser->getFd());
	std::ostringstream invitingMsg;
	invitingMsg << ":ft_irc " << std::setw(3) << std::setfill('0') << RPL_INVITING
				<< " " << m_user->getNickname() << " "
				<< targetNick << " " << channelName;
	m_responseHandler.sendResponse(m_user->getFd(), invitingMsg.str());

	std::string inviteMsg = m_user->getPrefix() + " INVITE " + targetNick + " :" + channelName;
	m_responseHandler.sendResponse(targetUser->getFd(), inviteMsg);

	#ifdef DEBUG
	std::cout << "User " << m_user->getNickname()
			  << " invited " << targetNick
			  << " to channel " << channelName << std::endl;
	#endif
}

// ==================== WHOIS Handler ====================

void Command::handleWHOIS()
{
	std::string targetNick;

	if (!Validation::validateWHOIS(m_user, m_params, targetNick, m_responseHandler))
		return;

	User* targetUser = nullptr;
	const std::map<int, User*>& users = m_server.getUsers();
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
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOSUCHNICK, m_user->getNickname(), targetNick + " :No such nick/channel");
		return;
	}

	std::ostringstream whoisUserMsg;
	whoisUserMsg << ":ft_irc " << std::setw(3) << std::setfill('0') << RPL_WHOISUSER
				 << " " << m_user->getNickname() << " "
				 << targetUser->getNickname() << " "
				 << targetUser->getUsername() << " "
				 << targetUser->getHostname() << " * :"
				 << targetUser->getRealname();
	m_responseHandler.sendResponse(m_user->getFd(), whoisUserMsg.str());

	const std::vector<std::string>& channels = targetUser->getChannels();
	if (!channels.empty())
	{
		std::ostringstream whoisChannelsMsg;
		whoisChannelsMsg << ":ft_irc " << std::setw(3) << std::setfill('0') << RPL_WHOISCHANNELS
						 << " " << m_user->getNickname() << " "
						 << targetUser->getNickname() << " :";

		for (size_t i = 0; i < channels.size(); ++i)
		{
			if (i > 0)
				whoisChannelsMsg << " ";
			whoisChannelsMsg << channels[i];
		}
		m_responseHandler.sendResponse(m_user->getFd(), whoisChannelsMsg.str());
	}

	std::ostringstream whoisServerMsg;
	whoisServerMsg << ":ft_irc " << std::setw(3) << std::setfill('0') << RPL_WHOISSERVER
				   << " " << m_user->getNickname() << " "
				   << targetUser->getNickname() << " ft_irc :ft_irc server";
	m_responseHandler.sendResponse(m_user->getFd(), whoisServerMsg.str());

	std::ostringstream endOfWhoisMsg;
	endOfWhoisMsg << ":ft_irc " << std::setw(3) << std::setfill('0') << RPL_ENDOFWHOIS
				  << " " << m_user->getNickname() << " "
				  << targetUser->getNickname() << " :End of /WHOIS list";
	m_responseHandler.sendResponse(m_user->getFd(), endOfWhoisMsg.str());

	#ifdef DEBUG
	std::cout << "User " << m_user->getNickname() << " requested WHOIS for " << targetNick << std::endl;
	#endif
}

// ==================== QUIT Handler ====================

void Command::handleQUIT()
{
	std::string quitMessage;

	if (!Validation::validateQUIT(m_user, m_params, quitMessage))
		return;

	std::string quitMsg = m_user->getPrefix() + " QUIT :Quit: " + quitMessage;

	m_responseHandler.sendResponse(m_user->getFd(), "ERROR :Closing connection");

	#ifdef DEBUG
	std::cout << "User " << m_user->getNickname() << " quit: " << quitMessage << std::endl;
	#endif

	m_server.removeClient(m_user->getFd(), quitMsg);
}

// [Takato]: added from here
// Edited by Dominique
void	Command::handleMODE()
{
	// Parameters: <channel> *( ( "-" / "+" ) *<modes> *<modeparams> )

	std::string		channelName {};
	Channel* 		channel { Validation::validateMODE(m_user, m_params, m_server, channelName, m_responseHandler) };

	if (!channel)
		return;

	if (m_params.size() == 1)
	{
		// Dominique:
		// NOTE: In Irssi, typing just '/mode' triggers client-side "Irssi: not enough parameters" warning, but
		// Irssi still auto-appends the current channel and sends "MODE #currentchannel".
		// However, Irssi then rejects/doesn't display the (correctly formatted) server response.
		// Unless you can find a solution this seems to be an irssi quirk we should just accept.
		m_responseHandler.sendNumericReply(m_user->getFd(), RPL_CHANNELMODEIS, m_user->getNickname(), channelName + " " + channel->getModeString());
		return;
	}

	if (!Validation::validateCanChangeModes(m_user, channel, channelName, m_responseHandler))
		return;

	const std::string	&modes{ m_params[1] };
	const char 			sign{ modes.front() };

	if (!Validation::validateModes(m_user, channelName, modes, m_responseHandler))
		return;

	int	modeSettingIdxOffset {};

	for (int i{ 1 }; i < static_cast<int>(std::min(modes.size(), k_max_mode_num + 1)); ++i)
	{
		if (!Validation::validateModeCharacter(m_user, channelName, modes[i], k_mode_set_param + k_mode_set_toggle, m_responseHandler))
			return;

		if (k_mode_set_toggle.find(modes[i]) != std::string::npos)
			modeOperateToggle(modes[i], sign);
		else if (k_mode_set_param.find(modes[i]) != std::string::npos)
		{
			if (m_params.size() < MINIMUM_PARAMS_MODE + 1 + static_cast<std::size_t>(modeSettingIdxOffset))
			{
				m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NEEDMOREPARAMS, m_user->getNickname(), "MODE :Not enough parameters");
				return;
			}
			try
			{
				modeOperateParam(modes[i], sign, modeSettingIdxOffset);
				if (!(sign == '-' && modes[i] == 'l'))
					modeSettingIdxOffset++;
			}
			catch ( IrcNumericCodes error_code )
			{
				Validation::handleModeOperationError(m_user, channelName, error_code, m_responseHandler);
				if (!(sign == '-' && modes[i] == 'l'))
					modeSettingIdxOffset++;
				continue;
			}
		}
	}

	#ifdef DEBUG
	m_server.printModeStates();
	#endif
}

void	Command::modeOperateToggle(char mode, char sign)
{
	switch (mode)
	{
	case 'i':
		modeOperateToggleInvite(sign);
		break ;
	case 't':
		modeOperateToggleTopic(sign);
		break ;
	default:
		break ;
	}
}

void	Command::modeOperateParam(char mode, char sign, int idxOffset)
{
	switch (mode)
	{
	case 'k':
		modeOperateParamKey(sign, idxOffset);
		break ;
	case 'o':
		modeOperateParamPrivilege(sign, idxOffset);
		break;
	case 'l':
		modeOperateParamLimit(sign, idxOffset);
		break;
	default:
		break;
	}
}

void	Command::modeOperateToggleInvite(char sign)
{
	const std::string	&channel_name { m_params[0] };
	Channel				*channel { m_server.getChannels().at(channel_name)};

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

void	Command::modeOperateToggleTopic(char sign)
{
	const std::string	&channel_name { m_params[0] };
	Channel				*channel { m_server.getChannels().at(channel_name)};

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

void	Command::modeOperateParamKey(char sign, int idxOffset)
{
	const std::string	&channel_name { m_params[0] };
	const std::string	&param { m_params[MINIMUM_PARAMS_MODE + idxOffset] };
	Channel				*channel { m_server.getChannels().at(channel_name) };

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

void	Command::modeOperateParamPrivilege(char sign, int idxOffset)
{
	const std::string	&channel_name { m_params[0] };
	const std::string	&param { m_params[MINIMUM_PARAMS_MODE + idxOffset] };
	Channel				*channel { m_server.getChannels().at(channel_name) };
	User				*user {};

	for (auto it{ m_server.getUsers().begin() }; it != m_server.getUsers().end(); ++it)
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
		channel->addOperator(m_user->getFd());
		break ;
	case '-':
		channel->removeOperator(m_user->getFd());
		break ;
	default:
		break ;
	}
}

void	Command::modeOperateParamLimit(char sign, int idxOffset)
{
	const std::string	&channel_name { m_params[0] };
	const std::string	&param { m_params[MINIMUM_PARAMS_MODE + idxOffset] };
	Channel				*channel { m_server.getChannels().at(channel_name) };

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
