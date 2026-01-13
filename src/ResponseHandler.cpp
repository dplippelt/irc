/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseHandler.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/13 15:47:35 by spyun             #+#    #+#             */
/*   Updated: 2026/01/13 13:25:48 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ResponseHandler.hpp"
#include "Server.hpp"

// =================== Constructor & Destructor ==================

ResponseHandler::ResponseHandler(Server& server)
	: m_server(server) {}

ResponseHandler::~ResponseHandler() {}


// ==================== Basic Message Sending ====================

void ResponseHandler::sendResponse(int fd, const std::string& message)
{
	std::string fullMessage = message;
	if (fullMessage.empty() || fullMessage.back() != '\n')
		fullMessage += "\r\n";

	#ifdef DEBUG
	std::cout << "Sent to fd " << fd << ": " << fullMessage;
	#endif
	m_server.sendToClient(fd, fullMessage);
}

// ==================== Numeric Reply Messages ====================

/* Send to status window */
void ResponseHandler::sendNumericReply(int fd, int code, const std::string& message)
{
	std::ostringstream oss;
	oss << ":ft_irc " << std::setw(3) << std::setfill('0') << code << " * " << message;
	sendResponse(fd, oss.str());

	#ifdef DEBUG
	std::cout << "Numeric reply " << code << " sent to fd " << fd << std::endl;
	#endif
}

/* Send to user if user is registered or status window if not registered */
void ResponseHandler::sendNumericReply(int fd, int code, const std::string& nickname,
										const std::string& message)
{
	std::string nick { nickname };
	if (nick.empty())
		nick = "*";

	std::ostringstream oss;
	oss << ":ft_irc " << std::setw(3) << std::setfill('0') << code
		<< " " << nick << " " << message;
	sendResponse(fd, oss.str());

	#ifdef DEBUG
	std::cout << "Numeric reply " << code << " sent to " << nickname
			  << " (fd " << fd << ")" << std::endl;
	#endif
}

// ==================== Error Messages ====================

void ResponseHandler::sendError(int fd, const std::string& command, const std::string& message)
{
	std::string errorMsg = "ERROR :" + command + " " + message;
	sendResponse(fd, errorMsg);
}

void ResponseHandler::sendAuthenticationError(int fd, const std::string& command)
{
	if (command == "NICK" || command == "USER")
		sendNumericReply(fd, ERR_NOTREGISTERED, ":You have not registered");
	else
		sendNumericReply(fd, ERR_NOTREGISTERED, ":You have not registered");
}

// ==================== Welcome Sequence ====================

void ResponseHandler::sendWelcome(User* user)
{
	int fd { user->getFd() };
	std::string nick { user->getNickname() };
	std::string prefix { user->getPrefix().substr(1) };

	sendNumericReply(fd, RPL_WELCOME, nick, ":Welcome to the Internet Relay Network " + prefix);
	sendNumericReply(fd, RPL_YOURHOST, nick, ":Your host is ft_irc, running version 1.0");
	sendNumericReply(fd, RPL_CREATED, nick, ":This server was created sometime");
	sendNumericReply(fd, RPL_MYINFO, nick, "ft_irc 1.0 o itkol");
	sendNumericReply(fd, RPL_ISUPPORT, nick, "CHANTYPES=# PREFIX=(o)@ CHANMODES=,k,l,it MODES NETWORK=ft_irc :are supported by this server");

	#ifdef DEBUG
	std::cout << "Welcome sequence sent to " << nick
			  << " (fd " << user->getFd() << ")" << std::endl;
	#endif
}

// ==================== JOIN Command Messages ====================

void ResponseHandler::sendJoinMessages(User* user, Channel* channel)
{
	std::string joinMsg = user->getPrefix() + " JOIN " + channel->getName();

	const std::map<int, User*>& members = channel->getMembers();
	for (std::map<int, User*>::const_iterator it = members.begin();
		 it != members.end(); ++it)
	{
		sendResponse(it->second->getFd(), joinMsg);
	}

	sendTopicMessage(user, channel);
	sendNameMessage(user, channel);

	#ifdef DEBUG
	std::cout << "JOIN messages sent to " << user->getNickname()
			  << " for channel " << channel->getName() << std::endl;
	#endif
}

void ResponseHandler::sendTopicMessage(User* user, Channel* channel)
{
	if (!channel->getTopic().empty())
	{
		std::ostringstream topicMsg;
		topicMsg << ":ft_irc " << std::setw(3) << std::setfill('0') << RPL_TOPIC
				 << " " << user->getNickname() << " "
				 << channel->getName() << " :" << channel->getTopic();
		sendResponse(user->getFd(), topicMsg.str());

		std::ostringstream topicWhoTimeMsg;
		topicWhoTimeMsg << ":ft_irc " << std::setw(3) << std::setfill('0') << RPL_TOPICWHOTIME
						<< " " << user->getNickname() << " "
						<< channel->getName() << " "
						<< channel->getTopicSetBy() << " "
						<< channel->getTopicSetTime();
		sendResponse(user->getFd(), topicWhoTimeMsg.str());
	}
}

void ResponseHandler::sendNameMessage(User* user, Channel* channel)
{
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
