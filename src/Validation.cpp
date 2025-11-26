/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Validation.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: dlippelt <dlippelt@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/13 15:41:37 by dlippelt      #+#    #+#                 */
/*   Updated: 2025/11/26 15:43:43 by seungah       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Validation.hpp"

/* ==================== Class Behavior ==================== */

bool Validation::validatePASS( User* user, const std::vector<std::string>& params )
{
	if ( user->isRegistered() )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_ALREADYREGISTRED, ":You may not reregister");
		return false;
	}
	if ( user->hasProvidedPassword() )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_ALREADYREGISTRED, ":You have already provided a password");
		return false;
	}
	if ( params.empty() )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "PASS :Not enough parameters");
		return false;
	}
	return true;
}

bool	Validation::validateNICK( User* user, const std::vector<std::string>& params, const Server& server, std::string& outNick )
{
	if ( params.empty() )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NONICKNAMEGIVEN, ":No nickname given");
		return false;
	}

	std::string newNick { params.front() };

	if ( !newNick.empty() && newNick[0] == ':' )
		newNick = newNick.substr(1);

	if ( !ValidationHelper::isValidNickname(newNick) )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_ERRONEUSNICKNAME, newNick + " :Erroneous nickname");
		return false;
	}
	if ( ValidationHelper::isNicknameInUse(newNick, server.getUsers()) )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NICKNAMEINUSE, newNick + " :Nickname is already in use");
		return false;
	}

	outNick = newNick;
	return true;
}

bool	Validation::validateUSER( User* user, const std::vector<std::string>& params )
{
	if ( user->isRegistered() )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_ALREADYREGISTRED, ":You may not reregister");
		return false;
	}
	if ( user->hasUsername() )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_ALREADYREGISTRED, ":You have already set a username");
		return false;
	}
	if ( params.size() < 4 )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "USER :Not enough parameters");
		return false;
	}
	return true;
}

bool	Validation::validateJOIN( User* user, const std::vector<std::string>& params )
{
	if ( !user->isRegistered() )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NOTREGISTERED, ":You have not registered");
		return false;
	}
	if ( params.empty() )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "JOIN :Not enough parameters");
		return false;
	}
	return true;
}

bool	Validation::validatePRIVMSG( User* user, const std::vector<std::string>& params )
{
	if ( !user->isRegistered() )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NOTREGISTERED, ":You have not registered");
		return false;
	}
	if ( params.empty() )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NORECIPIENT, "PRIVMSG :No recipient given");
		return false;
	}
	if ( params.size() < 2 )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "PRIVMSG :Not enough parameters");
		return false;
	}
	return true;
}

bool	Validation::validateKICK( User* user, const std::vector<std::string>& params, std::string& targetNick, std::string& channelName, std::string& reason )
{
	if ( !user->isRegistered() )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NOTREGISTERED, ":You have not registered");
		return false;
	}
	if ( params.size() < 2 )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "KICK :Not enough parameters");
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

bool	Validation::validatePART( User* user, const std::vector<std::string>& params )
{
	if ( !user->isRegistered() )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NOTREGISTERED, ":You have not registered");
		return false;
	}
	if ( params.empty() )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "PART :Not enough parameters");
		return false;
	}
	return true;
}

bool	Validation::validateTOPIC( User* user, const std::vector<std::string>& params, std::string& outChannelName )
{
	if ( !user->isRegistered() )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NOTREGISTERED, ":You have not registered");
		return false;
	}
	if ( params.empty() )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "TOPIC :Not enough parameters");
		return false;
	}

	std::string	channelName { params.front() };

	if ( !channelName.empty() && channelName[0] == ':' )
		channelName = channelName.substr(1);

	outChannelName = channelName;
	return true;
}

bool	Validation::validateINVITE( User* user, const std::vector<std::string>& params, std::string& targetNick, std::string& channelName )
{
	if ( !user->isRegistered() )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NOTREGISTERED, ":You have not registered");
		return false;
	}
	if ( params.size() < 2 )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NEEDMOREPARAMS, "INVITE :Not enough parameters");
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

bool	Validation::validateCanJoin( User* user, Channel* channel, std::string& channelKey )
{

	if ( channel->isMember(user->getFd()) )
		return false;

	if ( channel->isInviteOnly() && !channel->isInvited(user->getFd()) )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_INVITEONLYCHAN, channel->getName() + " :Cannot join channel (+i)");
		return false;
	}
	if ( channel->hasKey() )
	{
		if ( channelKey.empty() || channelKey != channel->getKey() )
		{
			ResponseHandler::sendNumericReply(user->getFd(), ERR_BADCHANNELKEY, channel->getName() + " :Cannot join channel (+k)");
			return false;
		}
	}
	if ( channel->hasUserLimit() )
	{
		if ( static_cast<int>(channel->getMemberCount()) >= channel->getUserLimit() )
		{
			ResponseHandler::sendNumericReply(user->getFd(), ERR_CHANNELISFULL, channel->getName() + " :Cannot join channel (+l)");
			return false;
		}
	}
	return true;
}

Channel*	Validation::validateCanSendMsg( User* user, const std::string& target, const Server& server )
{
	auto it { server.getChannels().find(target) };

	if ( it == server.getChannels().end() )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NOSUCHCHANNEL, target + " :No such channel");
		return nullptr;
	}

	Channel* channel { it->second };

	if ( !channel->isMember(user->getFd()) )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_CANNOTSENDTOCHAN, target + " :Cannot send to channel");
		return nullptr;
	}

	return channel;
}

Channel*	Validation::validateCanKick( User* user, const std::string& channelName, const Server& server )
{
	auto chanIt { server.getChannels().find(channelName) };

	if ( chanIt == server.getChannels().end() )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NOSUCHCHANNEL, channelName + " :No such channel");
		return nullptr;
	}

	Channel* channel { chanIt->second };

	if ( !channel->isMember(user->getFd()) )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
		return nullptr;
	}
	if ( !channel->isOperator(user->getFd()) )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
		return nullptr;
	}

	return channel;
}

User*	Validation::validateCanKickTarget( User* user, Channel* channel, const std::string& targetNick, const Server& server )
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
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NOSUCHNICK, targetNick + " :No such nick");
		return nullptr;
	}

	if ( !channel->isMember(targetUser->getFd()) )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_USERNOTINCHANNEL, targetNick + " " + channel->getName() + " :They aren't on that channel");
		return nullptr;
	}

	return targetUser;
}

Channel*	Validation::validateCanPart( User* user, const std::string& currentChannel, const Server& server )
{
	auto chanIt { server.getChannels().find(currentChannel) };

	if ( chanIt == server.getChannels().end() )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NOSUCHCHANNEL, currentChannel + " :No such channel");
		return nullptr;
	}

	Channel* channel { chanIt->second };

	if ( !channel->isMember(user->getFd()) )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NOTONCHANNEL, currentChannel + " :You're not on that channel");
		return nullptr;
	}

	return channel;
}

Channel*	Validation::validateCanChangeTopic( User* user, const std::string& channelName, const Server& server )
{
	auto chanIt { server.getChannels().find(channelName) };

	if ( chanIt == server.getChannels().end() )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NOSUCHCHANNEL, channelName + " :No such channel");
		return nullptr;
	}

	Channel* channel { chanIt->second };

	if ( !channel->isMember(user->getFd()) )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
		return nullptr;
	}

	return channel;
}

Channel*	Validation::validateCanInvite( User* user, const std::string& channelName, const Server& server )
{
	auto chanIt { server.getChannels().find(channelName) };

	if ( chanIt == server.getChannels().end() )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NOSUCHCHANNEL, channelName + " :No such channel");
		return nullptr;
	}

	Channel* channel { chanIt->second };

	if ( !channel->isMember(user->getFd()) )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
		return nullptr;
	}
	if ( channel->isInviteOnly() && !channel->isOperator(user->getFd()) )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
		return nullptr;
	}

	return channel;
}

User*	Validation::validateCanInviteTarget( User* user, Channel* channel, const std::string& channelName, const std::string& targetNick, const Server& server )
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
		ResponseHandler::sendNumericReply(user->getFd(), ERR_NOSUCHNICK, targetNick + " :No such nick");
		return nullptr;
	}
	if ( channel->isMember(targetUser->getFd()) )
	{
		ResponseHandler::sendNumericReply(user->getFd(), ERR_USERONCHANNEL, targetNick + " " + channelName + " :is already on channel");
		return nullptr;
	}

	return targetUser;
}
