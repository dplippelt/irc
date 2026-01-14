/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmitsuya <tmitsuya@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 17:16:17 by spyun             #+#    #+#             */
/*   Updated: 2026/01/14 13:55:38 by tmitsuya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Commands.hpp"
#include "Server.hpp"
#include "Message.hpp"
#include "CTCPHandler.hpp"

// =================== Constructor & Destructor ==================

Command::Command(Server& server, User* user, Message& msg)
	: m_server { server }
	, m_responseHandler { server }
	, m_user { user }
	, m_command { msg.getCommandName() }
	, m_params { msg.getParamsList() }
{
}

Command::~Command() = default;

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
	if ( !Validation::validatePASS(m_user, m_params, m_responseHandler) )
		return;

	std::string providedPassword { ValidationHelper::removeLeadingColon(m_params.front()) };
	if ( !ValidationHelper::isValidPassword(providedPassword) )
		return m_responseHandler.sendNumericReply(m_user->getFd(), ERR_PASSWDMISMATCH, m_user->getNickname(), ":Password contains invalid characters");
	if ( !Authentication::validatePassword(providedPassword, m_server.getPassword()) )
		return m_responseHandler.sendNumericReply(m_user->getFd(), ERR_PASSWDMISMATCH, m_user->getNickname(), ":Password incorrect");

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

	if ( !Validation::validateNICK(m_user, m_params, m_server, newNick, m_responseHandler) )
		return;

	std::string oldNick { m_user->getNickname() };
	std::string oldPrefix { m_user->getPrefix() };
	m_user->setNickname(newNick);
	m_user->setHasNickname(true);

	Authentication::checkRegistration(m_user, m_responseHandler);

	if ( Authentication::isRegistered(m_user) && !oldNick.empty() )
		informUsersOfNickChange(oldPrefix, newNick);

	#ifdef DEBUG
	std::cout << "User fd " << m_user->getFd()
			  << " changed nickname from '" << oldNick << "' to '" << newNick << "'" << std::endl;
	#endif
}

void Command::informUsersOfNickChange( const std::string& oldPrefix, const std::string& newNick )
{
	std::string nickChangeMsg { oldPrefix + " NICK :" + newNick };
	int			userFd { m_user->getFd() };

	// send to the user changing their nick separately in case they are not in a channel
	m_responseHandler.sendResponse(userFd, nickChangeMsg);

	// inform all other users that share a channel with the user that is changing ther nick
	std::set<int>	informedFds { userFd };
	auto			channelList { m_server.getChannels() };

	for ( auto it { channelList.begin() }; it != channelList.end(); ++it )
	{
		const Channel* channel { it->second };

		if ( !channel->isMember(userFd) )
			continue;

		auto chanMembers { channel->getMembers() };

		for ( auto itt { chanMembers.begin() }; itt != chanMembers.end(); ++itt )
		{
			int	otherUserFd { itt->second->getFd() };

			if ( informedFds.find(otherUserFd) == informedFds.end() )
			{
				m_responseHandler.sendResponse(otherUserFd, nickChangeMsg);
				informedFds.insert(otherUserFd);
			}
		}
	}
}

// ==================== USER Command ====================

void Command::handleUSER()
{
	if ( !Validation::validateUSER(m_user, m_params, m_responseHandler) )
		return;

	auto it { m_params.begin() };
	std::string username { *it++ };
	std::advance(it, 2);
	std::string realname { ValidationHelper::removeLeadingColon(*it) };

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
	std::string pong_str {};

	if ( !m_params.empty() )
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
	if ( !Validation::validateJOIN(m_user, m_params, m_responseHandler) )
		return;

	std::map<std::string, Channel*>& channels { m_server.getChannels() };
	const std::vector<std::string> channelVec { getChannelVector() };
	const std::vector<std::string> keys { getKeyVector() };

	for ( size_t i = 0; i < channelVec.size(); ++i )
	{
		std::string currentChannel { channelVec[i] };
		std::string channelKey { (i < keys.size()) ? keys[i] : "" };

		if ( !ValidationHelper::isValidChannelName(currentChannel) )
		{
			m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOSUCHCHANNEL, m_user->getNickname(), currentChannel + " :No such channel");
			continue;
		}

		Channel* channel { getOrCreateChannel(currentChannel, channels) };

		if ( !Validation::validateCanJoin(m_user, channel, channelKey, m_responseHandler) )
			continue;

		channel->addMember(m_user);
		m_user->joinChannel(currentChannel);

		if ( channel->isInvited(m_user->getFd()) )
			channel->removeInvite(m_user->getFd());

		m_responseHandler.sendJoinMessages(m_user, channel);

		#ifdef DEBUG
		std::cout << "User " << m_user->getNickname()
				  << " joined channel " << currentChannel << std::endl;
		#endif
	}
}

const std::vector<std::string> Command::getKeyVector() const
{
	std::string keyList {};
	if ( m_params.size() > 1 )
	{
		auto it { std::next(m_params.begin(), 1) };
		keyList = ValidationHelper::removeLeadingColon(*it);
	}

	std::vector<std::string> keys {};
	if ( !keyList.empty() )
	{
		std::istringstream keyStream(keyList);
		std::string key;

		while ( std::getline(keyStream, key, ',') )
			keys.push_back(key);
	}

	return keys;
}

Channel* Command::getOrCreateChannel(const std::string& channelName, std::map<std::string, Channel*>& channels)
{
	auto it { channels.find(channelName) };
	if (it != channels.end())
		return it->second;

	Channel* newChannel { new Channel(channelName) };
	channels[channelName] = newChannel;

	#ifdef DEBUG
	std::cout << "Created new channel: " << channelName << std::endl;
	#endif

	return newChannel;
}

// ==================== PRIVMSG Handler ====================

void Command::handlePRIVMSG()
{
	if ( !Validation::validatePRIVMSG(m_user, m_params, m_responseHandler) )
		return;

	auto it { m_params.begin() };
	std::string target { *it++ };
	std::string message { *it };

	if ( !message.empty() )
		message = message[0] == ':' ? (message.substr(1) + "\r\n") : (message + "\r\n");

	#ifdef DEBUG
	CTCPHandler::debugDCC(m_user, target, message.substr(0, message.length() - 2));
	#endif

	if ( target[0] == '#' || target[0] == '&' )
		return sendPrivMsgToChannel(target, message);
	sendPrivMsgToUser(target, message);
}

void Command::sendPrivMsgToChannel(const std::string& target, const std::string& message)
{
	Channel* channel { Validation::validateCanSendMsg(m_user, target, m_server, m_responseHandler) };
	if ( !channel )
		return;

	std::string privmsgToChannel { m_user->getPrefix() + " PRIVMSG " + target + " :" + message };
	channel->broadcast(privmsgToChannel, m_server, m_user->getFd());

	#ifdef DEBUG
	std::cout << "User " << m_user->getNickname() << " sent message to channel " << target << std::endl;
	#endif
}

void Command::sendPrivMsgToUser(const std::string& target, const std::string& message)
{
	const User* targetUser { getTargetUser(target) };
	if ( targetUser == nullptr )
		return m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOSUCHNICK, m_user->getNickname(), target + " :No such nick/channel");

	std::string privmsgToUser { m_user->getPrefix() + " PRIVMSG " + target + " :" + message };
	m_server.sendToClient(targetUser->getFd(), privmsgToUser);

	#ifdef DEBUG
	std::cout << "User " << m_user->getNickname() << " sent DM to " << target << ": " << message << std::endl;
	#endif
}

// ==================== KICK Handler ====================

void Command::handleKICK()
{
	std::string	targetNick {};
	std::string	channelName {};

	if ( !Validation::validateKICK(m_user, m_params, targetNick, channelName, m_responseHandler) )
		return;

	Channel* channel { Validation::validateCanKick(m_user, channelName, m_server, m_responseHandler) };
	if( !channel )
		return;

	User* targetUser { Validation::validateCanKickTarget(m_user, channel, targetNick, m_server, m_responseHandler) };
	if( !targetUser )
		return;

	sendKickResponse(channel, channelName, targetNick);

	channel->removeMember(targetUser->getFd());
	targetUser->leaveChannel(channelName);

	if ( channel->isEmpty() )
		removeEmptyChannel(channel, channelName);

	#ifdef DEBUG
	std::cout << "User " << targetNick << " was kicked from channel " << channelName << " by " << m_user->getNickname() << std::endl;
	#endif
}

const std::string Command::getKickReason() const
{
	if ( m_params.size() == 2 )
		return m_user->getNickname();

	auto it { std::next(m_params.begin(), 2) };
	std::string reason { *it };

	if ( !reason.empty() && reason[0] == ':' )
		if ( !reason.substr(1).empty() )
			return reason.substr(1);

	return m_user->getNickname();
}

void Command::sendKickResponse(Channel* channel, const std::string& channelName, const std::string& targetNick)
{
	std::string kickMsg { m_user->getPrefix() + " KICK " + channelName + " " + targetNick + " :" + getKickReason() + "\r\n" };
	channel->broadcast(kickMsg, m_server);
}

// ==================== PART Handler ====================

void Command::handlePART()
{
	if ( !Validation::validatePART(m_user, m_params, m_responseHandler) )
		return;

	for ( const auto& currentChannel : getChannelVector() )
	{
		Channel* channel { Validation::validateCanPart(m_user, currentChannel, m_server, m_responseHandler) };
		if (!channel)
			continue;

		sendPartResponse(channel, currentChannel);

		channel->removeMember(m_user->getFd());
		m_user->leaveChannel(currentChannel);

		if ( channel->isEmpty() )
			removeEmptyChannel(channel, currentChannel);

		#ifdef DEBUG
		std::cout << "User " << m_user->getNickname() << " left channel " << currentChannel;
		if (!getPartReason().empty())
			std::cout << " (reason: " << getPartReason() << ")";
		std::cout << std::endl;
		#endif
	}
}

const std::string Command::getPartReason() const
{
	if ( m_params.size() <= 1 )
		return "";

	auto it { std::next(m_params.begin(), 1) };
	std::string reason { *it };

	if ( !reason.empty() && reason[0] == ':' )
		reason = reason.substr(1);
	for ( ++it; it != m_params.end(); ++it )
		reason += " " + *it;

	return reason;
}

const std::string Command::getPartMessage(const std::string& currentChannel) const
{
	std::string partMsg { m_user->getPrefix() + " PART " + currentChannel };

	std::string reason { getPartReason() };
	if ( !reason.empty() )
		partMsg += " :" + reason;

	return partMsg + "\r\n";
}

void Command::sendPartResponse(Channel* channel, const std::string& currentChannel)
{
	std::string partMsg { getPartMessage(currentChannel) };
	channel->broadcast(partMsg, m_server);
}

// ==================== TOPIC Handler ====================

void Command::handleTOPIC()
{
	std::string	channelName {};

	if ( !Validation::validateTOPIC(m_user, m_params, channelName, m_responseHandler) )
		return;

	Channel* channel { Validation::validateCanChangeTopic(m_user, channelName, m_server, m_responseHandler) };
	if( !channel )
		return;

	if ( m_params.size() == 1 )
		return m_responseHandler.sendCurrentTopicResponse(m_user, channel, channelName);

	if ( channel->isTopicRestricted() && !channel->isOperator(m_user->getFd()) )
		return m_responseHandler.sendNumericReply(m_user->getFd(), ERR_CHANOPRIVSNEEDED, m_user->getNickname(), channelName + " :You're not channel operator");

	std::string newTopic { getNewTopic() };

	channel->setTopic(newTopic);
	channel->setTopicSetBy(m_user->getPrefix().substr(1));
	channel->setTopicSetTime();

	m_responseHandler.sendTopicChangeResponse(m_user, channel, channelName, newTopic);
}

const std::string Command::getNewTopic() const
{
	auto it { std::next(m_params.begin(), 1) };
	std::string newTopic { *it };

	if ( !newTopic.empty() && newTopic[0] == ':' )
		newTopic = newTopic.substr(1);

	for ( ++it; it != m_params.end(); ++it )
		newTopic += " " + *it;

	return newTopic;
}

// ==================== INVITE Handler ====================

void Command::handleINVITE()
{
	std::string	targetNick {};
	std::string	channelName {};

	if ( !Validation::validateINVITE(m_user, m_params, targetNick, channelName, m_responseHandler) )
		return;

	Channel* channel { Validation::validateCanInvite(m_user, channelName, m_server, m_responseHandler) };
	if( !channel )
		return;

	User* targetUser { Validation::validateCanInviteTarget(m_user, channel, channelName, targetNick, m_server, m_responseHandler) };
	if( !targetUser )
		return;

	channel->addInvite(targetUser->getFd());

	m_responseHandler.sendInviteResponses(m_user, targetUser, targetNick, channelName);

	#ifdef DEBUG
	std::cout << "User " << m_user->getNickname()
			  << " invited " << targetNick
			  << " to channel " << channelName << std::endl;
	#endif
}

// ==================== WHOIS Handler ====================

void Command::handleWHOIS()
{
	std::string targetNick {};

	if ( !Validation::validateWHOIS(m_user, m_params, targetNick, m_responseHandler) )
		return;

	const User* targetUser { getTargetUser(targetNick) };
	if ( targetUser == nullptr )
		return m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOSUCHNICK, m_user->getNickname(), targetNick + " :No such nick/channel");

	m_responseHandler.sendWhoIsUserResponse(m_user, targetUser);

	const std::vector<std::string>& channels { targetUser->getChannels() };
	if ( !channels.empty() )
		m_responseHandler.sendWhoIsChannelsResponse(m_user, targetUser, channels);

	m_responseHandler.sendWhoIsServerResponse(m_user, targetUser);
	m_responseHandler.sendEndOfWhoIsResponse(m_user, targetUser);

	#ifdef DEBUG
	std::cout << "User " << m_user->getNickname() << " requested WHOIS for " << targetNick << std::endl;
	#endif
}

// ==================== QUIT Handler ====================

void Command::handleQUIT()
{
	std::string quitMsg { m_user->getPrefix() + " QUIT " + getQuitReason(m_params) };

	m_responseHandler.sendResponse(m_user->getFd(), "ERROR :Closing connection");

	#ifdef DEBUG
	std::cout << "User " << m_user->getNickname() << " quit: " << getQuitReason(m_params) << std::endl;
	#endif

	m_server.removeClient(m_user->getFd(), quitMsg);
}

const std::string Command::getQuitReason( const std::vector<std::string>& params ) const
{
	if ( params.empty() )
		return ":Client Quit\r\n";

	std::string quitMessage {};
	if ( !quitMessage.empty() && quitMessage[0] == ':' )
		quitMessage = quitMessage.substr(1);

	if ( quitMessage.empty() )
		return ":Client Quit\r\n";

	for ( size_t i = 1; i < params.size(); ++i )
		quitMessage += " " + params[i];

	return ":Quit: " + quitMessage + "\r\n";
}

// ==================== MODE Handler ====================

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
		m_responseHandler.sendNumericReply(
			m_user->getFd(), RPL_CHANNELMODEIS, m_user->getNickname(),
			channelName + " " + channel->getModeString()
		);
		return;
	}

	if (!Validation::validateCanChangeModes(m_user, channel, channelName, m_responseHandler))
		return;

	std::vector<t_mode_elems>	mode_param_pairs{};
	std::vector<t_mode_elems>	mode_param_pairs_done{};
	std::string 				response{};

	if (createSignModePairs(mode_param_pairs, m_params[1]))
		return ;
	assignParamsToModes(mode_param_pairs);
	if (checkModeparamPairValidation(mode_param_pairs))
		return ;
	
	for (auto &pair : mode_param_pairs)
	{
		if (!changeChannelMode(channel, pair))
			mode_param_pairs_done.push_back(pair);
	}
	// for safety reason
	if (mode_param_pairs_done.empty())
		return ;
	response = generateModeResponse(channel, mode_param_pairs_done);
	channel->broadcast(response, m_server);

	#ifdef DEBUG
	m_server.printModeStates();
	#endif
}

std::string	Command::generateModeResponse(Channel* channel, const std::vector<t_mode_elems> &mode_param_pairs)
{
	std::string	response{};
	std::string	modes_done{};
	std::string	params_done{};
	char		current_sign{};

	for (auto &pair : mode_param_pairs)
	{
		if (current_sign != pair.sign)
		{
			current_sign = pair.sign;
			modes_done.push_back(pair.sign);
		}
		modes_done.push_back(pair.mode);
		if (!pair.param.empty())
		{
			params_done += pair.param + " ";
		}
	}
	if (!params_done.empty())
		params_done.pop_back();
	response += m_user->getPrefix() + " MODE " + channel->getName() + " ";
	response += modes_done + " " + params_done + "\r\n";
	return response;
}

int	Command::createSignModePairs(std::vector<t_mode_elems> &mode_param_pairs, const std::string &signs_modes)
{
	t_mode_elems	mode_param_pair{};

	for (char c : signs_modes)
	{
		if (k_mode_available.find(c) == std::string::npos)
		{
			if (c == '+' || c == '-')
			{
				mode_param_pair.sign = c;
			}
			else
			{
				m_responseHandler.sendNumericReply(
					m_user->getFd(), ERR_UNKNOWNMODE, m_user->getNickname(),
					std::string(1, c) + " :is unknown mode char to me"
				);
				return 1;
			}
		}
		else
		{
			if (mode_param_pair.sign)
			{
				mode_param_pair.mode = c;
				mode_param_pairs.push_back(mode_param_pair);
			}
			else
			{
				m_responseHandler.sendNumericReply(
					m_user->getFd(), ERR_UNKNOWNMODE, m_user->getNickname(),
					std::string(1, c) + " :is unknown mode char to me"
				);
				return 1;
			}
		}
	}
	return 0;
}

void		Command::assignParamsToModes(std::vector<t_mode_elems> &mode_param_pairs)
{
	const std::size_t	nparam{ m_params.size() };
	std::size_t			i{ 2 };

	for (auto &pair : mode_param_pairs)
	{
		if (k_modes_without_param.find(pair.mode) != std::string::npos)
			continue ;
		if (pair.mode == 'l' && pair.sign == '-')
			continue ;
		if (i < nparam)
			pair.param = m_params[i++];
	}
}

int	Command::checkModeparamPairValidation(const std::vector<t_mode_elems> &mode_param_pairs)
{
	for (auto &pair : mode_param_pairs)
	{
		switch (pair.mode)
		{
		case 'k':
		case 'o':
			if (pair.param.empty())
			{
				m_responseHandler.sendNumericReply(
					m_user->getFd(), ERR_NEEDMOREPARAMS, m_user->getNickname(),
					"MODE :Not enough parameters"
				);
				return 1;
			}
			break ;
		case 'l':
			if (pair.sign == '+')
			{
				if (pair.param.empty())
				{
					m_responseHandler.sendNumericReply(
						m_user->getFd(), ERR_NEEDMOREPARAMS, m_user->getNickname(),
						"MODE :Not enough parameters"
					);
					return 1;
				}
				try
				{
					if (std::stoi(pair.param) < 0)
					{
						m_responseHandler.sendNumericReply(
							m_user->getFd(), ERR_UNKNOWNMODE, m_user->getNickname(),
							std::string(1, pair.param.front()) + " :is unknown mode char to me"
						);
						return 1;
					}
				}
				catch(const std::exception& e)
				{
					m_responseHandler.sendNumericReply(
						m_user->getFd(), ERR_UNKNOWNMODE, m_user->getNickname(),
						std::string(1, pair.param.front()) + " :is unknown mode char to me"
					);
					return 1;
				}
			}
			break ;
		default:
			break ;
		}
	}
	return 0;
}

int		Command::changeChannelMode(Channel* channel, const t_mode_elems &mode_param_pairs)
{
	switch (mode_param_pairs.mode)
	{
	case 'i':
		return changeChannelModeInvite(channel, mode_param_pairs);
	case 't':
		return changeChannelModeTopic(channel, mode_param_pairs);
	case 'k':
		return changeChannelModeKey(channel, mode_param_pairs);
	case 'o':
		return changeChannelModePrivilege(channel, mode_param_pairs);
	case 'l':
		return changeChannelModeLimit(channel, mode_param_pairs);
	default:
		break;
	}
	return 1;
}

int	Command::changeChannelModeInvite(Channel* channel, const t_mode_elems &mode_param_pairs)
{
	switch (mode_param_pairs.sign)
	{
	case '+':
		if (!channel->isInviteOnly())
		{
			channel->setInviteOnly(true);
			return 0;
		}
		break ;
	case '-':
		if (channel->isInviteOnly())
		{
			channel->setInviteOnly(false);
			return 0;
		}
		break ;
	default:
		break ;
	}
	return 1;
}

int	Command::changeChannelModeTopic(Channel* channel, const t_mode_elems &mode_param_pairs)
{
	switch (mode_param_pairs.sign)
	{
	case '+':
		if (!channel->isTopicRestricted())
		{
			channel->setTopicRestricted(true);
			return 0;
		}
		break ;
	case '-':
		if (channel->isTopicRestricted())
		{
			channel->setTopicRestricted(false);
			return 0;
		}
		break ;
	default:
		break ;
	}
	return 1;
}

int	Command::changeChannelModeKey(Channel* channel, const t_mode_elems &mode_param_pairs)
{
	switch (mode_param_pairs.sign)
	{
	case '+':
		channel->setKey(mode_param_pairs.param);
		channel->setHasKey(true);
		return 0;
	case '-':
		if (channel->hasKey())
		{
			channel->setHasKey(false);
			return 0;
		}
		break ;
	default:
		break ;
	}
	return 1;
}

int	Command::changeChannelModePrivilege(Channel* channel, const t_mode_elems &mode_param_pairs)
{
	const User	*target {};

	for (auto &member : channel->getMembers())
	{
		if (member.second->getNickname() == mode_param_pairs.param)
		{
			target = member.second;
			break ;
		}
	}
	if (!target)
	{
		m_responseHandler.sendNumericReply(
			m_user->getFd(), ERR_USERNOTINCHANNEL, m_user->getNickname(),
			target->getNickname() + " " + channel->getName() + " :They aren't on that channel"
		);
		return 1;
	}
	switch (mode_param_pairs.sign)
	{
	case '+':
		channel->addOperator(m_user->getFd());
		return 0;
	case '-':
		channel->removeOperator(m_user->getFd());
		return 0;
	default:
		break ;
	}
	return 1;
}

int	Command::changeChannelModeLimit(Channel* channel, const t_mode_elems &mode_param_pairs)
{
	switch (mode_param_pairs.sign)
	{
	case '+':
		channel->setUserLimit(std::stoi(mode_param_pairs.param));
		channel->setHasUserLimit(true);
		return 0;
	case '-':
		if (channel->hasUserLimit())
		{
			channel->setHasUserLimit(false);
			return 0;
		}
		break ;
	default:
		break ;
	}
	return 1;
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


// ==================== Misc Helpers ====================

Command::CommandType Command::getCmdType() const
{
	auto it { k_commands.find(m_command) };

	return ( it != k_commands.end() ? it->second : CMD_UNKNOWN );
}

const User* Command::getTargetUser(const std::string& targetNick) const
{
	const std::map<int, User*>& users { m_server.getUsers() };

	for ( auto it = users.begin(); it != users.end(); ++it )
		if (it->second->getNickname() == targetNick)
			return it->second;

	return nullptr;
}

const std::vector<std::string> Command::getChannelVector() const
{
	std::string channelList = ValidationHelper::removeLeadingColon(m_params.front());

	std::vector<std::string> channelVec;
	std::istringstream channelStream(channelList);
	std::string channelName;
	while (std::getline(channelStream, channelName, ','))
	{
		if (!channelName.empty())
			channelVec.push_back(channelName);
	}
	return channelVec;
}

void Command::removeEmptyChannel(Channel* channel, const std::string& channelName)
{
	m_server.getChannels().erase(channelName);
	delete channel;

	#ifdef DEBUG
	std::cout << "Channel " << channelName << " deleted as it became empty." << std::endl;
	#endif
}
