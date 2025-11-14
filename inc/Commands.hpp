/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Commands.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/28 11:10:22 by spyun         #+#    #+#                 */
/*   Updated: 2025/11/14 19:43:15 by spyun         ########   odam.nl         */
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
		Commands() = delete;
		Commands(const Commands&) = delete;
		Commands& operator=(const Commands&) = delete;
		~Commands() = delete;

		static void sendResponse(int fd, const std::string& message);
		static bool isValidNickname(const std::string& nick);
		static bool isNicknameInUse(const std::string& nick, const std::map<int, User*>& users);
		static void checkRegistration(User* user);

		static void sendWelcome(User* user);
		static void sendError(int fd, const std::string& command, const std::string& message);
		static void sendNumericReply(int fd, int code, const std::string& message);

		static bool isValidChannelName(const std::string& channelName);
		static Channel* getOrCreateChannel(const std::string& channelName, std::map<std::string, Channel*>& channels);
		static void sendJoinMessages(User* user, Channel* channel);

		static bool canExecuteCommand(User* user, const std::string& command);
		static void sendAuthenticationError(int fd, const std::string& command);

	public:
		static void executeCommand(User* user, const std::string& command,
							const std::vector<std::string>& params,
							std::map<int, User*>& users,
							std::map<std::string, Channel*>& channels,
							const std::string& serverPassword);

		static void handlePASS(User* user, const std::list<std::string>& params, const std::string& serverPassword);
		static void handleNICK(User* user, const std::list<std::string>& params, const std::map<int, User*>& users);
		static void handleUSER(User* user, const std::list<std::string>& params);
		static void handleJOIN(User* user, const std::list<std::string>& params, std::map<std::string, Channel*>& channels);
		static void handlePRIVMSG(User* user, const std::list<std::string>& params, const std::map<int, User*>& users, std::map<std::string, Channel*>& channels);
		static void handleKICK(User* user, const std::list<std::string>& params, const std::map<int, User*>& users, std::map<std::string, Channel*>& channels);
		static void handlePART(User* user, const std::list<std::string>& params, std::map<std::string, Channel*>& channels);
		static void handleTOPIC(User* user, const std::list<std::string>& params, std::map<std::string, Channel*>& channels);
		static void handleINVITE(User* user, const std::list<std::string>& params, const std::map<int, User*>& users, std::map<std::string, Channel*>& channels);

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
		static const int ERR_TOOMANYCHANNELS = 405;
		static const int ERR_NORECIPIENT = 411;
		static const int ERR_NOTEXTTOSEND = 412;
		static const int ERR_CANNOTSENDTOCHAN = 404;
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
		static const int ERROR_CHANOPRIVSNEEDED = 482;
};

#endif
