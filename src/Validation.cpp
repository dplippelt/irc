/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Validation.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: dlippelt <dlippelt@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/13 15:41:37 by dlippelt      #+#    #+#                 */
/*   Updated: 2025/12/11 16:22:39 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Validation.hpp"

/* ==================== Class Behavior ==================== */

bool Validation::validatePASS( User* user, const std::vector<std::string>& params, ResponseHandler& responseHandler)
{
	if ( user->isRegistered() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_ALREADYREGISTRED, ":You may not reregister");
		return false;
	}
	if ( user->hasProvidedPassword() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_ALREADYREGISTRED, ":You have already provided a password");
		return false;
	}
	if ( params.empty() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "PASS :Not enough parameters");
		return false;
	}
	return true;
}

bool	Validation::validateNICK( User* user, const std::vector<std::string>& params, const Server& server, std::string& outNick, ResponseHandler& responseHandler )
{
	if ( params.empty() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NONICKNAMEGIVEN, ":No nickname given");
		return false;
	}

	std::string newNick { params.front() };

	if ( !newNick.empty() && newNick[0] == ':' )
		newNick = newNick.substr(1);

	if ( !ValidationHelper::isValidNickname(newNick) )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_ERRONEUSNICKNAME, newNick + " :Erroneous nickname");
		return false;
	}
	if ( ValidationHelper::isNicknameInUse(newNick, server.getUsers()) )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NICKNAMEINUSE, newNick + " :Nickname is already in use");
		return false;
	}

	outNick = newNick;
	return true;
}

bool	Validation::validateUSER( User* user, const std::vector<std::string>& params, ResponseHandler& responseHandler )
{
	if ( user->isRegistered() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_ALREADYREGISTRED, ":You may not reregister");
		return false;
	}
	if ( user->hasUsername() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_ALREADYREGISTRED, ":You have already set a username");
		return false;
	}
	if ( params.size() < 4 )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "USER :Not enough parameters");
		return false;
	}
	return true;
}

bool	Validation::validateJOIN( User* user, const std::vector<std::string>& params, ResponseHandler& responseHandler )
{
	if ( !user->isRegistered() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NOTREGISTERED, ":You have not registered");
		return false;
	}
	if ( params.empty() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "JOIN :Not enough parameters");
		return false;
	}
	return true;
}

bool	Validation::validatePRIVMSG( User* user, const std::vector<std::string>& params, ResponseHandler& responseHandler )
{
	if ( !user->isRegistered() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NOTREGISTERED, ":You have not registered");
		return false;
	}
	if ( params.empty() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NORECIPIENT, "PRIVMSG :No recipient given");
		return false;
	}
	if ( params.size() < 2 )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "PRIVMSG :Not enough parameters");
		return false;
	}
	return true;
}

bool	Validation::validateKICK( User* user, const std::vector<std::string>& params, std::string& targetNick, std::string& channelName, std::string& reason, ResponseHandler& responseHandler )
{
	if ( !user->isRegistered() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NOTREGISTERED, ":You have not registered");
		return false;
	}
	if ( params.size() < 2 )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "KICK :Not enough parameters");
		return false;
	}

	auto it { params.begin() };

	channelName = *it++;
	targetNick = *it++;
	reason = "Kicked by operator";

	if ( it != params.end() )
	{
		reason = *it;
		if ( !reason.empty() && reason[0] == ':' )
			reason = reason.substr(1);
	}
	return true;
}

bool	Validation::validatePART( User* user, const std::vector<std::string>& params, ResponseHandler& responseHandler )
{
	if ( !user->isRegistered() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NOTREGISTERED, ":You have not registered");
		return false;
	}
	if ( params.empty() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "PART :Not enough parameters");
		return false;
	}
	return true;
}

bool	Validation::validateTOPIC( User* user, const std::vector<std::string>& params, std::string& outChannelName, ResponseHandler& responseHandler )
{
	if ( !user->isRegistered() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NOTREGISTERED, ":You have not registered");
		return false;
	}
	if ( params.empty() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "TOPIC :Not enough parameters");
		return false;
	}

	std::string	channelName { params.front() };

	if ( !channelName.empty() && channelName[0] == ':' )
		channelName = channelName.substr(1);

	outChannelName = channelName;
	return true;
}

bool	Validation::validateINVITE( User* user, const std::vector<std::string>& params, std::string& targetNick, std::string& channelName, ResponseHandler& responseHandler )
{
	if ( !user->isRegistered() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NOTREGISTERED, ":You have not registered");
		return false;
	}
	if ( params.size() < 2 )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "INVITE :Not enough parameters");
		return false;
	}

	auto it { params.begin() };

	targetNick = *it++;
	channelName = *it;

	if ( !channelName.empty() && channelName[0] == ':' )
		channelName = channelName.substr(1);
	if ( !targetNick.empty() && targetNick[0] == ':' )
		targetNick = targetNick.substr(1);
	return true;
}

bool	Validation::validateQUIT( User* user, const std::vector<std::string>& params, std::string& quitMessage )
{
	(void)user;

	if ( params.empty() )
	{
		quitMessage = "Client exited";
		return true;
	}

	quitMessage = params[0];
	if ( !quitMessage.empty() && quitMessage[0] == ':' )
		quitMessage = quitMessage.substr(1);

	for ( size_t i = 1; i < params.size(); ++i )
		quitMessage += " " + params[i];

	return true;
}

bool	Validation::validateWHOIS( User* user, const std::vector<std::string>& params, std::string& targetNick, ResponseHandler& responseHandler )
{
	if ( params.empty() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NONICKNAMEGIVEN, ":No nickname given");
		return false;
	}

	targetNick = params[0];
	if ( !targetNick.empty() && targetNick[0] == ':' )
		targetNick = targetNick.substr(1);

	return true;
}

Channel*	Validation::validateMODE( User* user, const std::vector<std::string>& params, const Server& server, std::string& channelName, ResponseHandler& responseHandler, ResponseHandler& responseHandler )
{
	if (params.empty())
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, user->getNickname(), "MODE :Not enough parameters");
		return nullptr;
	}

	channelName = params[0];

	if (channelName[0] != '#' && channelName[0] != '+')
		return nullptr; // we do not implement user mode changes, only channel mode changes - so do nothing in this case

	auto it { server.getChannels().find(channelName) };
	if ( it == server.getChannels().end() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NOSUCHCHANNEL, user->getNickname(), channelName + " :No such channel");
		return nullptr;
	}

	Channel* channel { it->second };

	if (channelName[0] == '+')
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NOCHANMODES, user->getNickname(), channelName + " :Channel doesn't support modes");
		return nullptr;
	}

	return channel;
}

bool	Validation::validateCanJoin( User* user, Channel* channel, std::string& channelKey, ResponseHandler& responseHandler )
{

	if ( channel->isMember(user->getFd()) )
		return false;

	if ( channel->isInviteOnly() && !channel->isInvited(user->getFd()) )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_INVITEONLYCHAN, channel->getName() + " :Cannot join channel (+i)");
		return false;
	}
	if ( channel->hasKey() )
	{
		if ( channelKey.empty() || channelKey != channel->getKey() )
		{
			responseHandler.sendNumericReply(user->getFd(), ERR_BADCHANNELKEY, channel->getName() + " :Cannot join channel (+k)");
			return false;
		}
	}
	if ( channel->hasUserLimit() )
	{
		if ( static_cast<int>(channel->getMemberCount()) >= channel->getUserLimit() )
		{
			responseHandler.sendNumericReply(user->getFd(), ERR_CHANNELISFULL, channel->getName() + " :Cannot join channel (+l)");
			return false;
		}
	}
	return true;
}

Channel*	Validation::validateCanSendMsg( User* user, const std::string& target, const Server& server, ResponseHandler& responseHandler )
{
	auto it { server.getChannels().find(target) };

	if ( it == server.getChannels().end() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NOSUCHCHANNEL, target + " :No such channel");
		return nullptr;
	}

	Channel* channel { it->second };

	if ( !channel->isMember(user->getFd()) )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_CANNOTSENDTOCHAN, target + " :Cannot send to channel");
		return nullptr;
	}

	return channel;
}

Channel*	Validation::validateCanKick( User* user, const std::string& channelName, const Server& server, ResponseHandler& responseHandler )
{
	auto chanIt { server.getChannels().find(channelName) };

	if ( chanIt == server.getChannels().end() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NOSUCHCHANNEL, channelName + " :No such channel");
		return nullptr;
	}

	Channel* channel { chanIt->second };

	if ( !channel->isMember(user->getFd()) )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
		return nullptr;
	}
	if ( !channel->isOperator(user->getFd()) )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
		return nullptr;
	}

	return channel;
}

User*	Validation::validateCanKickTarget( User* user, Channel* channel, const std::string& targetNick, const Server& server, ResponseHandler& responseHandler )
{
	User* targetUser { nullptr };

	for ( auto it = server.getUsers().begin(); it != server.getUsers().end(); ++it )
	{
		if ( it->second->getNickname() == targetNick )
		{
			targetUser = it->second;
			break;
		}
	}

	if ( !targetUser )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NOSUCHNICK, targetNick + " :No such nick");
		return nullptr;
	}

	if ( !channel->isMember(targetUser->getFd()) )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_USERNOTINCHANNEL, targetNick + " " + channel->getName() + " :They aren't on that channel");
		return nullptr;
	}

	return targetUser;
}

Channel*	Validation::validateCanPart( User* user, const std::string& currentChannel, const Server& server, ResponseHandler& responseHandler )
{
	auto chanIt { server.getChannels().find(currentChannel) };

	if ( chanIt == server.getChannels().end() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NOSUCHCHANNEL, currentChannel + " :No such channel");
		return nullptr;
	}

	Channel* channel { chanIt->second };

	if ( !channel->isMember(user->getFd()) )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NOTONCHANNEL, currentChannel + " :You're not on that channel");
		return nullptr;
	}

	return channel;
}

Channel*	Validation::validateCanChangeTopic( User* user, const std::string& channelName, const Server& server, ResponseHandler& responseHandler )
{
	auto chanIt { server.getChannels().find(channelName) };

	if ( chanIt == server.getChannels().end() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NOSUCHCHANNEL, channelName + " :No such channel");
		return nullptr;
	}

	Channel* channel { chanIt->second };

	if ( !channel->isMember(user->getFd()) )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
		return nullptr;
	}

	return channel;
}

Channel*	Validation::validateCanInvite( User* user, const std::string& channelName, const Server& server, ResponseHandler& responseHandler )
{
	auto chanIt { server.getChannels().find(channelName) };

	if ( chanIt == server.getChannels().end() )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NOSUCHCHANNEL, channelName + " :No such channel");
		return nullptr;
	}

	Channel* channel { chanIt->second };

	if ( !channel->isMember(user->getFd()) )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
		return nullptr;
	}
	if ( channel->isInviteOnly() && !channel->isOperator(user->getFd()) )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
		return nullptr;
	}

	return channel;
}

User*	Validation::validateCanInviteTarget( User* user, Channel* channel, const std::string& channelName, const std::string& targetNick, const Server& server, ResponseHandler& responseHandler, ResponseHandler& responseHandler )
{
	User* targetUser { nullptr };

	for ( auto it = server.getUsers().begin(); it != server.getUsers().end(); ++it )
	{
		if ( it->second->getNickname() == targetNick )
		{
			targetUser = it->second;
			break;
		}
	}

	if ( !targetUser )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NOSUCHNICK, targetNick + " :No such nick");
		return nullptr;
	}
	if ( channel->isMember(targetUser->getFd()) )
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_USERONCHANNEL, targetNick + " " + channelName + " :is already on channel");
		return nullptr;
	}

	return targetUser;
}

bool	Validation::validateCanChangeModes( User* user, Channel* channel, const std::string& channelName, ResponseHandler& responseHandler )
{
	if (!user->isInChannel(channelName))
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_NOTONCHANNEL, user->getNickname(), channelName + " :You're not on that channel");
		return false;
	}

	if (!channel->isOperator(user->getFd()))
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_CHANOPRIVSNEEDED, user->getNickname(), channelName + " :You're not channel operator");
		return false;
	}

	return true;
}

bool	Validation::validateModes(User *user, const std::string &modes, ResponseHandler& responseHandler)
{
	// Dominique
	// NOTE: This might reject "MODE #channel +" which some servers actually allow apparently (and just doesn't do anything)
	if (modes.size() < 2)
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_UNKNOWNMODE, user->getNickname(), modes + " :is unknown mode char to me");
		return false;
	}

	if (!(modes.front() == '+' || modes.front() == '-'))
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_UNKNOWNMODE, user->getNickname(), std::string(1, modes.front())  + " :is unknown mode char to me");
		return false;
	}

	return true;
}

bool	Validation::validateModeCharacter( User* user, char mode, const std::string& availableModes, ResponseHandler& responseHandler )
{
	if (availableModes.find(mode) == std::string::npos)
	{
		responseHandler.sendNumericReply(user->getFd(), ERR_UNKNOWNMODE, std::string(1, mode) + " :is unknown mode char to me");
		return false;
	}
	return true;
}

void	Validation::handleModeOperationError( User* user, const std::string& channelName, IrcNumericCodes error_code, ResponseHandler& responseHandler )
{
	switch (error_code)
	{
	case ERR_KEYSET:
		responseHandler.sendNumericReply(user->getFd(), ERR_KEYSET, channelName + " :Channel key already set");
		break;
	case ERR_NOTONCHANNEL:
		responseHandler.sendNumericReply(user->getFd(), ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
		break;
	case ERR_NEEDMOREPARAMS:
		responseHandler.sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "MODE :Not enough parameters");
		break;
	default:
		responseHandler.sendError(user->getFd(), "MODE", "Unknown error occurred");
		break;
	}
}
