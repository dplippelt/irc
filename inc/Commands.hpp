/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Commands.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/28 11:10:22 by spyun         #+#    #+#                 */
/*   Updated: 2025/11/07 09:49:03 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>
#include <list>
#include <map>
#include <sstream>
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <iomanip>
#include "User.hpp"
#include "Channel.hpp"

class Commands
{
	private:
		std::map<int, User*>& _users;
		std::map<std::string, Channel*>& _channels;
		std::string _serverPassword;

		void sendResponse(int fd, const std::string& message);
		bool isValidNickname(const std::string& nick) const;
		bool isNicknameInUse(const std::string& nick) const;
		void checkRegistration(User* user);

		void sendWelcome(User* user);
		void sendError(int fd, const std::string& command, const std::string& message);
		void sendNumericReply(int fd, int code, const std::string& message);

		bool isValidChannelName(const std::string& channelName) const;
		Channel* getOrCreateChannel(const std::string& channelName);
		void sendJoinMessages(User* user, Channel* channel);

		bool canExecuteCommand(User* user, const std::string& command) const;
		void sendAuthenticationError(int fd, const std::string& command);

	public:
		Commands(std::map<int, User*>& users,
				 std::map<std::string, Channel*>& channels,
				 const std::string& password);
		~Commands();

		void executeCommand(User* user, const std::string& command,
							const std::vector<std::string>& params);

		// Authentication commands
		void handlePASS(User* user, const std::list<std::string>& params);
		void handleNICK(User* user, const std::list<std::string>& params);
		void handleUSER(User* user, const std::list<std::string>& params);

		// Channel commands
		void handleJOIN(User* user, const std::list<std::string>& params);

		// Messaging commands
		void handlePRIVMSG(User* user, const std::list<std::string>& params);

		// IRC Numeric Reply Codes
		static const int RPL_WELCOME = 001;
		static const int RPL_YOURHOST = 002;
		static const int RPL_CREATED = 003;
		static const int RPL_MYINFO = 004;
		static const int RPL_TOPIC = 332;
		static const int RPL_NAMREPLY = 353;
		static const int RPL_ENDOFNAMES = 366;

		static const int ERR_NOSUCHNICK = 401;
		static const int ERR_NOSUCHCHANNEL = 403;
		static const int ERR_TOOMANYCHANNELS = 405;
		static const int ERR_NORECIPIENT = 411;
		static const int ERR_NOTEXTTOSEND = 412;
		static const int ERR_CANNOTSENDTOCHAN = 404;
		static const int ERR_NONICKNAMEGIVEN = 431;
		static const int ERR_ERRONEUSNICKNAME = 432;
		static const int ERR_NICKNAMEINUSE = 433;
		static const int ERR_NOTONCHANNEL = 442;
		static const int ERR_NOTREGISTERED = 451;
		static const int ERR_NEEDMOREPARAMS = 461;
		static const int ERR_ALREADYREGISTRED = 462;
		static const int ERR_PASSWDMISMATCH = 464;
		static const int ERR_CHANNELISFULL = 471;
		static const int ERR_INVITEONLYCHAN = 473;
		static const int ERR_BADCHANNELKEY = 475;
};

#endif
