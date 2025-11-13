/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validation.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/13 15:41:37 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/13 17:48:27 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Validation.hpp"

/* ==================== Class Behavior ==================== */

void Validation::validatePASS(User* user, const std::list<std::string>& params)
{
	if (user->isRegistered())
	{
		Commands::sendNumericReply(user->getFd(), ERR_ALREADYREGISTRED, ":You may not reregister");
		throw std::exception {};
	}
	if (user->hasProvidedPassword())
	{
		Commands::sendNumericReply(user->getFd(), ERR_ALREADYREGISTRED, ":You have already provided a password");
		throw std::exception {};
	}
	if (params.empty())
	{
		Commands::sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "PASS :Not enough parameters");
		throw std::exception {};
	}
}

std::string	Validation::validateNICK(User* user, const std::list<std::string>& params, const Server& server)
{
	if (params.empty())
	{
		Commands::sendNumericReply(user->getFd(), ERR_NONICKNAMEGIVEN, ":No nickname given");
		throw std::exception {};
	}

	std::string newNick = params.front();
	if (!newNick.empty() && newNick[0] == ':')
		newNick = newNick.substr(1);

	if (!isValidNickname(newNick))
	{
		Commands::sendNumericReply(user->getFd(), ERR_ERRONEUSNICKNAME, newNick + " :Erroneous nickname");
		throw std::exception {};
	}
	if (isNicknameInUse(newNick, server))
	{
		Commands::sendNumericReply(user->getFd(), ERR_NICKNAMEINUSE, newNick + " :Nickname is already in use");
		throw std::exception {};
	}

	return newNick;
}

void	Validation::validateUSER(User* user, const std::list<std::string>& params)
{
	if (user->isRegistered())
	{
		Commands::sendNumericReply(user->getFd(), ERR_ALREADYREGISTRED, ":You may not reregister");
		throw std::exception {};
	}
	if (user->hasUsername())
	{
		Commands::sendNumericReply(user->getFd(), ERR_ALREADYREGISTRED, ":You have already set a username");
		throw std::exception {};
	}
	if (params.size() < 4)
	{
		Commands::sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "USER :Not enough parameters");
		throw std::exception {};
	}
}

void	Validation::validateJOIN(User* user, const std::list<std::string>& params)
{
	if (!user->isRegistered())
	{
		Commands::sendNumericReply(user->getFd(), ERR_NOTREGISTERED, ":You have not registered");
		throw std::exception {};
	}
	if (params.empty())
	{
		Commands::sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "JOIN :Not enough parameters");
		throw std::exception {};
	}
}

void	Validation::validatePRIVMSG(User* user, const std::list<std::string>& params)
{
	if (!user->isRegistered())
	{
		Commands::sendNumericReply(user->getFd(), ERR_NOTREGISTERED, ":You have not registered");
		throw std::exception {};
	}
	if (params.empty())
	{
		Commands::sendNumericReply(user->getFd(), ERR_NORECIPIENT, "PRIVMSG :No recipient given");
		throw std::exception {};
	}
	if (params.size() < 2)
	{
		Commands::sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "PRIVMSG :Not enough parameters");
		throw std::exception {};
	}
}

void	Validation::validateKICK(User* user, const std::list<std::string>& params)
{
	if (!user->isRegistered())
	{
		Commands::sendNumericReply(user->getFd(), ERR_NOTREGISTERED, ":You have not registered");
		throw std::exception {};
	}
	if (params.size() < 2)
	{
		Commands::sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "KICK :Not enough parameters");
		throw std::exception {};
	}
}

void	Validation::validateCanJoin(User* user, Channel* channel, std::string& channelKey)
{

	if (channel->isMember(user->getFd()))
		throw std::exception {};

	if (channel->isInviteOnly() && !channel->isInvited(user->getFd()))
	{
		Commands::sendNumericReply(user->getFd(), ERR_INVITEONLYCHAN, channel->getName() + " :Cannot join channel (+i)");
		throw std::exception {};
	}
	if (channel->hasKey())
	{
		if (channelKey.empty() || channelKey != channel->getKey())
		{
			Commands::sendNumericReply(user->getFd(), ERR_BADCHANNELKEY, channel->getName() + " :Cannot join channel (+k)");
			throw std::exception {};
		}
	}
	if (channel->hasUserLimit())
	{
		if (static_cast<int>(channel->getMemberCount()) >= channel->getUserLimit())
		{
			Commands::sendNumericReply(user->getFd(), ERR_CHANNELISFULL, channel->getName() + " :Cannot join channel (+l)");
			throw std::exception {};
		}
	}
}

Channel*	Validation::validateCanSendMsg(User* user, const std::string& target, const Server& server)
{
	auto it = server.getChannels().find(target);

	if (it == server.getChannels().end())
	{
		Commands::sendNumericReply(user->getFd(), ERR_NOSUCHCHANNEL, target + " :No such channel");
		throw std::exception {};
	}

	Channel* channel = it->second;

	if (!channel->isMember(user->getFd()))
	{
		Commands::sendNumericReply(user->getFd(), ERR_CANNOTSENDTOCHAN, target + " :Cannot send to channel");
		throw std::exception {};
	}

	return channel;
}

Channel*	Validation::validateCanKick(User* user, const std::string& channelName, const Server& server)
{
	auto chanIt = server.getChannels().find(channelName);

	if (chanIt == server.getChannels().end())
	{
		Commands::sendNumericReply(user->getFd(), ERR_NOSUCHCHANNEL, channelName + " :No such channel");
		throw std::exception {};
	}

	Channel* channel = chanIt->second;

	if (!channel->isMember(user->getFd()))
	{
		Commands::sendNumericReply(user->getFd(), ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
		throw std::exception {};
	}
	if (!channel->isOperator(user->getFd()))
	{
		Commands::sendNumericReply(user->getFd(), ERROR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
		throw std::exception {};
	}
}

User*	Validation::validateCanKickTarget(User* user, Channel* channel, const std::string& targetNick, const Server& server)
{
	User* targetUser {};

	for (auto it = server.getUsers().begin(); it != server.getUsers().end(); ++it)
	{
		if (it->second->getNickname() == targetNick)
		{
			targetUser = it->second;
			break;
		}
	}

	if (!channel->isMember(targetUser->getFd()))
	{
		Commands::sendNumericReply(user->getFd(), ERR_USERNOTINCHANNEL, targetNick + " " + channel->getName() + " :They aren't on that channel");
		throw std::exception {};
	}
}


/* ==================== Helpers ==================== */

bool Validation::isValidNickname(const std::string& nick)
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

bool Validation::isNicknameInUse(const std::string& nick, const Server& server)
{
	for (auto it = server.getUsers().begin(); it != server.getUsers().end(); ++it)
	{
		if (it->second->getNickname() == nick)
			return true;
	}
	return false;
}


