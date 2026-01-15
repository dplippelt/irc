/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validator.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 13:03:29 by dlippelt          #+#    #+#             */
/*   Updated: 2026/01/15 14:09:00 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Validator.hpp"

/* ==================== Constructor ==================== */

Validator::~Validator() = default;

Validator::Validator(Server& server, User* user, const std::vector<std::string>& params)
	: m_server { server }
	, m_responseHandler { server }
	, m_user { user }
	, m_params { params }
{
}

/* ==================== Class Behavior ==================== */

bool Validator::validatePASS()
{
	if ( m_user->isRegistered() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_ALREADYREGISTRED, m_user->getNickname(), ":You may not reregister");
		return false;
	}
	if ( m_user->hasProvidedPassword() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_ALREADYREGISTRED, m_user->getNickname(), ":You have already provided a password");
		return false;
	}
	if ( m_params.empty() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NEEDMOREPARAMS, m_user->getNickname(), "PASS :Not enough parameters");
		return false;
	}
	return true;
}

bool	Validator::validateNICK( std::string& outNick )
{
	if ( m_params.empty() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NONICKNAMEGIVEN, m_user->getNickname(), ":No nickname given");
		return false;
	}

	std::string newNick { m_params.front() };

	if ( !newNick.empty() && newNick[0] == ':' )
		newNick = newNick.substr(1);

	if ( !isValidNickname(newNick) )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_ERRONEUSNICKNAME, m_user->getNickname(), newNick + " :Erroneous Nickname");
		return false;
	}
	if ( isNicknameInUse(newNick, m_server.getUsers()) )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NICKNAMEINUSE, m_user->getNickname(), newNick + " :Nickname is already in use");
		return false;
	}

	outNick = newNick;
	return true;
}

bool	Validator::validateUSER()
{
	if ( m_user->isRegistered() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_ALREADYREGISTRED, m_user->getNickname(), ":You may not reregister");
		return false;
	}
	if ( m_user->hasUsername() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_ALREADYREGISTRED, m_user->getNickname(), ":You have already set a m_username");
		return false;
	}
	if ( m_params.size() < 4 || m_params[3].empty() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NEEDMOREPARAMS, m_user->getNickname(), "USER :Not enough parameters");
		return false;
	}
	return true;
}

bool	Validator::validateJOIN()
{
	if ( !m_user->isRegistered() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOTREGISTERED, m_user->getNickname(), ":You have not registered");
		return false;
	}
	if ( m_params.empty() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NEEDMOREPARAMS, m_user->getNickname(), "JOIN :Not enough parameters");
		return false;
	}
	return true;
}

bool	Validator::validatePRIVMSG()
{
	if ( !m_user->isRegistered() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOTREGISTERED, m_user->getNickname(), ":You have not registered");
		return false;
	}
	if ( m_params.empty() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NORECIPIENT, m_user->getNickname(), "PRIVMSG :No recipient given");
		return false;
	}
	if ( m_params.size() < 2 )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NEEDMOREPARAMS, m_user->getNickname(), "PRIVMSG :Not enough parameters");
		return false;
	}
	return true;
}

bool	Validator::validateKICK( std::string& targetNick, std::string& channelName )
{
	if ( !m_user->isRegistered() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOTREGISTERED, m_user->getNickname(), ":You have not registered");
		return false;
	}
	if ( m_params.size() < 2 )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NEEDMOREPARAMS, m_user->getNickname(), "KICK :Not enough parameters");
		return false;
	}

	auto it { m_params.begin() };

	channelName = *it++;
	targetNick = *it;

	return true;
}

bool	Validator::validatePART()
{
	if ( !m_user->isRegistered() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOTREGISTERED, m_user->getNickname(), ":You have not registered");
		return false;
	}
	if ( m_params.empty() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NEEDMOREPARAMS, m_user->getNickname(), "PART :Not enough parameters");
		return false;
	}
	return true;
}

bool	Validator::validateTOPIC( std::string& outChannelName )
{
	if ( !m_user->isRegistered() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOTREGISTERED, m_user->getNickname(), ":You have not registered");
		return false;
	}
	if ( m_params.empty() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NEEDMOREPARAMS, m_user->getNickname(), "TOPIC :Not enough parameters");
		return false;
	}

	std::string	channelName { m_params.front() };

	if ( !channelName.empty() && channelName[0] == ':' )
		channelName = channelName.substr(1);

	outChannelName = channelName;
	return true;
}

bool	Validator::validateINVITE( std::string& targetNick, std::string& channelName )
{
	if ( !m_user->isRegistered() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOTREGISTERED, m_user->getNickname(), ":You have not registered");
		return false;
	}
	if ( m_params.size() < 2 )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NEEDMOREPARAMS, m_user->getNickname(), "INVITE :Not enough parameters");
		return false;
	}

	auto it { m_params.begin() };

	targetNick = *it++;
	channelName = *it;

	if ( !channelName.empty() && channelName[0] == ':' )
		channelName = channelName.substr(1);
	if ( !targetNick.empty() && targetNick[0] == ':' )
		targetNick = targetNick.substr(1);
	return true;
}

bool	Validator::validateWHOIS( std::string& targetNick )
{
	if ( m_params.empty() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NONICKNAMEGIVEN, m_user->getNickname(), ":No nickname given");
		return false;
	}

	targetNick = m_params[0];
	if ( !targetNick.empty() && targetNick[0] == ':' )
		targetNick = targetNick.substr(1);

	return true;
}

Channel*	Validator::validateMODE( std::string& channelName )
{
	if (m_params.empty())
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NEEDMOREPARAMS, m_user->getNickname(), "MODE :Not enough parameters");
		return nullptr;
	}

	channelName = m_params[0];

	if (channelName[0] != '#' && channelName[0] != '+' && channelName.substr(0, m_user->getUsername().length()) == m_user->getUsername())
		return nullptr; // we do not implement user mode changes, only channel mode changes - so do nothing in this case

	auto it { m_server.getChannels().find(channelName) };
	if ( it == m_server.getChannels().end() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOSUCHCHANNEL, m_user->getNickname(), channelName + " :No such channel");
		return nullptr;
	}

	Channel* channel { it->second };

	if (channelName[0] == '+')
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOCHANMODES, m_user->getNickname(), channelName + " :Channel doesn't support modes");
		return nullptr;
	}

	return channel;
}

bool	Validator::validateCanJoin( Channel* channel, std::string& channelKey )
{

	if ( channel->isMember(m_user->getFd()) )
		return false;

	if ( channel->isInviteOnly() && !channel->isInvited(m_user->getFd()) )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_INVITEONLYCHAN, m_user->getNickname(), channel->getName() + " :Cannot join channel (+i)");
		return false;
	}
	if ( channel->hasKey() )
	{
		if ( channelKey.empty() || channelKey != channel->getKey() )
		{
			m_responseHandler.sendNumericReply(m_user->getFd(), ERR_BADCHANNELKEY, m_user->getNickname(), channel->getName() + " :Cannot join channel (+k)");
			return false;
		}
	}
	if ( channel->hasUserLimit() )
	{
		if ( static_cast<int>(channel->getMemberCount()) >= channel->getUserLimit() )
		{
			m_responseHandler.sendNumericReply(m_user->getFd(), ERR_CHANNELISFULL, m_user->getNickname(), channel->getName() + " :Cannot join channel (+l)");
			return false;
		}
	}
	return true;
}

Channel*	Validator::validateCanSendMsg( const std::string& target )
{
	auto it { m_server.getChannels().find(target) };

	if ( it == m_server.getChannels().end() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOSUCHCHANNEL, m_user->getNickname(), target + " :No such channel");
		return nullptr;
	}

	Channel* channel { it->second };

	if ( !channel->isMember(m_user->getFd()) )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_CANNOTSENDTOCHAN, m_user->getNickname(), target + " :Cannot send to channel");
		return nullptr;
	}

	return channel;
}

Channel*	Validator::validateCanKick( const std::string& channelName )
{
	auto chanIt { m_server.getChannels().find(channelName) };

	if ( chanIt == m_server.getChannels().end() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOSUCHCHANNEL, m_user->getNickname(), channelName + " :No such channel");
		return nullptr;
	}

	Channel* channel { chanIt->second };

	if ( !channel->isMember(m_user->getFd()) )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOTONCHANNEL, m_user->getNickname(), channelName + " :You're not on that channel");
		return nullptr;
	}
	if ( !channel->isOperator(m_user->getFd()) )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_CHANOPRIVSNEEDED, m_user->getNickname(), channelName + " :You're not channel operator");
		return nullptr;
	}

	return channel;
}

User*	Validator::validateCanKickTarget( Channel* channel, const std::string& targetNick )
{
	User* targetUser { nullptr };

	for ( auto it = m_server.getUsers().begin(); it != m_server.getUsers().end(); ++it )
	{
		if ( it->second->getNickname() == targetNick )
		{
			targetUser = it->second;
			break;
		}
	}

	if ( !targetUser )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOSUCHNICK, m_user->getNickname(), targetNick + " :No such nick");
		return nullptr;
	}

	if ( !channel->isMember(targetUser->getFd()) )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_USERNOTINCHANNEL, m_user->getNickname(), targetNick + " " + channel->getName() + " :They aren't on that channel");
		return nullptr;
	}

	return targetUser;
}

Channel*	Validator::validateCanPart( const std::string& currentChannel )
{
	auto chanIt { m_server.getChannels().find(currentChannel) };

	if ( chanIt == m_server.getChannels().end() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOSUCHCHANNEL, m_user->getNickname(), currentChannel + " :No such channel");
		return nullptr;
	}

	Channel* channel { chanIt->second };

	if ( !channel->isMember(m_user->getFd()) )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOTONCHANNEL, m_user->getNickname(), currentChannel + " :You're not on that channel");
		return nullptr;
	}

	return channel;
}

Channel*	Validator::validateCanChangeTopic( const std::string& channelName )
{
	auto chanIt { m_server.getChannels().find(channelName) };

	if ( chanIt == m_server.getChannels().end() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOSUCHCHANNEL, m_user->getNickname(), channelName + " :No such channel");
		return nullptr;
	}

	Channel* channel { chanIt->second };

	if ( !channel->isMember(m_user->getFd()) )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOTONCHANNEL, m_user->getNickname(), channelName + " :You're not on that channel");
		return nullptr;
	}

	return channel;
}

Channel*	Validator::validateCanInvite( const std::string& channelName )
{
	auto chanIt { m_server.getChannels().find(channelName) };

	if ( chanIt == m_server.getChannels().end() )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOSUCHCHANNEL, m_user->getNickname(), channelName + " :No such channel");
		return nullptr;
	}

	Channel* channel { chanIt->second };

	if ( !channel->isMember(m_user->getFd()) )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOTONCHANNEL, m_user->getNickname(), channelName + " :You're not on that channel");
		return nullptr;
	}
	if ( channel->isInviteOnly() && !channel->isOperator(m_user->getFd()) )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_CHANOPRIVSNEEDED, m_user->getNickname(), channelName + " :You're not channel operator");
		return nullptr;
	}

	return channel;
}

User*	Validator::validateCanInviteTarget( Channel* channel, const std::string& channelName, const std::string& targetNick )
{
	User* targetUser { nullptr };

	for ( auto it = m_server.getUsers().begin(); it != m_server.getUsers().end(); ++it )
	{
		if ( it->second->getNickname() == targetNick )
		{
			targetUser = it->second;
			break;
		}
	}

	if ( !targetUser )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOSUCHNICK, m_user->getNickname(), targetNick + " :No such nick");
		return nullptr;
	}
	if ( channel->isMember(targetUser->getFd()) )
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_USERONCHANNEL, m_user->getNickname(), targetNick + " " + channelName + " :is already on channel");
		return nullptr;
	}

	return targetUser;
}

bool	Validator::validateCanChangeModes( Channel* channel, const std::string& channelName )
{
	if (!m_user->isInChannel(channelName))
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOTONCHANNEL, m_user->getNickname(), channelName + " :You're not on that channel");
		return false;
	}

	if (!channel->isOperator(m_user->getFd()))
	{
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_CHANOPRIVSNEEDED, m_user->getNickname(), channelName + " :You're not channel operator");
		return false;
	}

	return true;
}

void	Validator::handleModeOperationError( const std::string& channelName, IrcNumericCodes error_code )
{
	switch (error_code)
	{
	case ERR_KEYSET:
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_KEYSET, m_user->getNickname(), channelName + " :Channel key already set");
		break;
	case ERR_NOTONCHANNEL:
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NOTONCHANNEL, m_user->getNickname(), channelName + " :You're not on that channel");
		break;
	case ERR_NEEDMOREPARAMS:
		m_responseHandler.sendNumericReply(m_user->getFd(), ERR_NEEDMOREPARAMS, m_user->getNickname(), "MODE :Not enough parameters");
		break;
	default:
		m_responseHandler.sendError(m_user->getFd(), "MODE", "Unknown error occurred");
		break;
	}
}

// ==================== Nickname Validation ====================

bool Validator::isValidNickname(const std::string& nick)
{
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
		{
			return false;
		}
	}
	return true;
}

bool Validator::isNicknameInUse(const std::string& nick, const std::map<int, User*>& users)
{
	for (std::map<int, User*>::const_iterator it = users.begin(); it != users.end(); ++it)
	{
		if (it->second->getNickname() == nick)
			return true;
	}
	return false;
}

bool Validator::isValidChannelName(const std::string& channelName)
{
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

// ==================== Password Validation ====================

bool Validator::isValidPassword(const std::string& password)
{
	if (password.empty())
		return false;
	for (size_t i = 0; i < password.length(); ++i)
	{
		char c = password[i];
		if (std::isspace(static_cast<unsigned char>(c)) || std::iscntrl(static_cast<unsigned char>(c)))
			return false;
	}
	return true;
}
