/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ResponseHandler.hpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/13 13:47:33 by spyun         #+#    #+#                 */
/*   Updated: 2025/11/24 09:38:43 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSEHANDLER_HPP
#define RESPONSEHANDLER_HPP

#include <string>
#include <sstream>
#include <iomanip>
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <cerrno>
#include "User.hpp"
#include "Channel.hpp"

class ResponseHandler
{
	private:
		ResponseHandler() = delete;
		ResponseHandler(const ResponseHandler&) = delete;
		ResponseHandler& operator=(const ResponseHandler&) = delete;
		~ResponseHandler() = delete;

		static void sendTopicMessage(User* user, Channel* channel);
		static void sendNameMessage(User* user, Channel* channel);

	public:
		static void sendResponse(int fd, const std::string& message);
		static void sendNumericReply(int fd, int code, const std::string& message);
		static void sendNumericReply(int fd, int code, const std::string& nickname, const std::string& message);
		static void sendError(int fd, const std::string& command, const std::string& message);
		static void sendAuthenticationError(int fd, const std::string& command);
		static void sendWelcome(User* user);
		static void sendJoinMessages(User* user, Channel* channel);

		// IRC Numeric Reply Codes
		static const int RPL_WELCOME = 001;
		static const int RPL_YOURHOST = 002;
		static const int RPL_CREATED = 003;
		static const int RPL_MYINFO = 004;
		static const int RPL_CHANNELMODEIS = 324;
		static const int RPL_NOTOPIC = 331;
		static const int RPL_TOPIC = 332;
		static const int RPL_INVITING = 341;
		static const int RPL_NAMREPLY = 353;
		static const int RPL_ENDOFNAMES = 366;

		static const int ERR_NOSUCHNICK = 401;
		static const int ERR_NOSUCHCHANNEL = 403;
		static const int ERR_CANNOTSENDTOCHAN = 404;
		static const int ERR_TOOMANYCHANNELS = 405;
		static const int ERR_NORECIPIENT = 411;
		static const int ERR_NOTEXTTOSEND = 412;
		static const int ERR_NONICKNAMEGIVEN = 431;
		static const int ERR_ERRONEUSNICKNAME = 432;
		static const int ERR_NICKNAMEINUSE = 433;
		static const int ERR_USERNOTINCHANNEL = 441;
		static const int ERR_NOTONCHANNEL = 442;
		static const int ERR_USERONCHANNEL = 443;
		static const int ERR_NOTREGISTERED = 451;
		static const int ERR_NEEDMOREPARAMS = 461;
		static const int ERR_ALREADYREGISTRED = 462;
		static const int ERR_PASSWDMISMATCH = 464;
		static const int ERR_KEYSET = 467;
		static const int ERR_CHANNELISFULL = 471;
		static const int ERR_UNKNOWNMODE = 472;
		static const int ERR_INVITEONLYCHAN = 473;
		static const int ERR_BADCHANNELKEY = 475;
		static const int ERR_CHANOPRIVSNEEDED = 482;
};

#endif
