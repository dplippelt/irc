/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ResponseHandler.cpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/13 15:47:35 by spyun         #+#    #+#                 */
/*   Updated: 2025/11/24 10:47:45 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "ResponseHandler.hpp"

// ==================== Basic Message Sending ====================

void ResponseHandler::sendResponse(int fd, const std::string& message)
{
	std::string fullMessage = message;
	if (fullMessage.empty() || fullMessage.back() != '\n')
		fullMessage += "\r\n";

	#ifdef DEBUG
	std::cout << "Sent to fd " << fd << ": " << fullMessage;
	#endif

	ssize_t sent = send(fd, fullMessage.c_str(), fullMessage.length(), 0);
	if (sent == -1)
	{
		std::cerr << "Error: Failed to send message to fd " << fd << ": "
				  << strerror(errno) << std::endl;
	}
}

// ==================== Numeric Reply Messages ====================

void ResponseHandler::sendNumericReply(int fd, int code, const std::string& message)
{
	std::ostringstream oss;
	oss << ":ft_irc " << std::setw(3) << std::setfill('0') << code << " * " << message;
	sendResponse(fd, oss.str());

	#ifdef DEBUG
	std::cout << "Numeric reply " << code << " sent to fd " << fd << std::endl;
	#endif
}

void ResponseHandler::sendNumericReply(int fd, int code, const std::string& nickname,
										const std::string& message)
{
	std::ostringstream oss;
	oss << ":ft_irc " << std::setw(3) << std::setfill('0') << code
		<< " " << nickname << " " << message;
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
	std::string nick = user->getNickname();
	std::string prefix = user->getPrefix();

	// RPL_WELCOME (001)
	std::ostringstream msg1;
	msg1 << ":ft_irc 001 " << nick << " :Welcome to the Internet Relay Network " << prefix;
	sendResponse(user->getFd(), msg1.str());

	// RPL_YOURHOST (002)
	std::ostringstream msg2;
	msg2 << ":ft_irc 002 " << nick << " :Your host is ft_irc, running version 1.0";
	sendResponse(user->getFd(), msg2.str());

	// RPL_CREATED (003)
	std::ostringstream msg3;
	msg3 << ":ft_irc 003 " << nick << " :This server was created sometime";
	sendResponse(user->getFd(), msg3.str());

	// RPL_MYINFO (004)
	std::ostringstream msg4;
	msg4 << ":ft_irc 004 " << nick << " ft_irc 1.0 o itkol";
	sendResponse(user->getFd(), msg4.str());

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
	}
}

void ResponseHandler::sendNameMessage(User* user, Channel* channel)
{
	// RPL_NAMREPLY (353)
	std::ostringstream namesMsg;
	namesMsg << ":ft_irc " << std::setw(3) << std::setfill('0') << RPL_NAMREPLY
			 << " " << user->getNickname() << " = " << channel->getName() << " :"
			 << channel->getMemberList();
	sendResponse(user->getFd(), namesMsg.str());

	// RPL_ENDOFNAMES (366)
	std::ostringstream endNamesMsg;
	endNamesMsg << ":ft_irc " << std::setw(3) << std::setfill('0') << RPL_ENDOFNAMES
				<< " " << user->getNickname() << " "
				<< channel->getName() << " :End of /NAMES list";
	sendResponse(user->getFd(), endNamesMsg.str());
}
