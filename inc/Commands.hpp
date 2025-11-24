/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 11:10:22 by spyun             #+#    #+#             */
/*   Updated: 2025/11/17 13:43:15 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDS_HPP
#define COMMANDS_HPP

# include <algorithm> // [Takato]: added
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
#include "Validation.hpp"
#include "Server.hpp"
#include "enums.hpp"

class Server;
class Validation;

class Server;
class Message;

class Commands
{
	private:
		std::map<int, User*>& _users;
		std::map<std::string, Channel*>& _channels;
		std::string _serverPassword;

		static void sendResponse(int fd, const std::string& message);
		// bool isValidNickname(const std::string& nick) const; // Dominique: moved to validation class
		// bool isNicknameInUse(const std::string& nick) const; // Dominique: moved to validation class
		void checkRegistration(User* user);

		void sendWelcome(User* user);
		void sendError(int fd, const std::string& command, const std::string& message);

		bool isValidChannelName(const std::string& channelName) const;
		Channel* getOrCreateChannel(const std::string& channelName);
		void sendJoinMessages(User* user, Channel* channel);

		bool canExecuteCommand(User* user, const std::string& command) const;
		void sendAuthenticationError(int fd, const std::string& command);

		static inline const size_t		k_max_mode_num{ 3 }; // [Takato]: added for mode operation
		static inline const std::string k_mode_set_toggle{ "it" }; // [Takato]: added for mode operation
		static inline const std::string k_mode_set_param{ "kol" }; // [Takato]: added for mode operation
		static void	modeOperateToggle(char mode, char sign, const Message &message, Server &server); // [Takato]: added for mode operation
		static void	modeOperateToggleInvite(char sign, const Message &message, Server &server); // [Takato]: added for mode operation
		static void	modeOperateToggleTopic(char sign, const Message &message, Server &server); // [Takato]: added for mode operation
		static void	modeOperateToggle(char mode, char sign, const Message &message, Server &server); // [Takato]: added for mode operation
		static void	modeOperateParam(char mode, char sign, const Message &message, Server &server); // [Takato]: added for mode operation
		static void	modeOperateParamPrivilege(char sign, const Message &message, Server &server); // [Takato]: added for mode operation
		static void	modeOperateParamKey(char sign, const Message &message, Server &server); // [Takato]: added for mode operation
		static void	modeOperateParamLimit(char sign, const Message &message, Server &server); // [Takato]: added for mode operation
		static void	channelValidation(const Server &server, User *user, const std::string &channel); // [Takato]: added for mode operation
		static void	modesValidation(const std::string &modes); // [Takato]: added for mode operation

	public:
		Commands(std::map<int, User*>& users,
				 std::map<std::string, Channel*>& channels,
				 const std::string& password);
		~Commands();

		static void sendNumericReply(int fd, int code, const std::string& message);

		void executeCommand(User* user, const std::string& command,
							const std::vector<std::string>& params, const Server& server);

		void handlePASS(User* user, const std::list<std::string>& params);
		void handleNICK(User* user, const std::list<std::string>& params, const Server& server);
		void handleUSER(User* user, const std::list<std::string>& params);
		void handleJOIN(User* user, const std::list<std::string>& params);
		void handlePRIVMSG(User* user, const std::list<std::string>& params, const Server& server);
		void handleKICK(User* user, const std::list<std::string>& params, const Server& server);
		void handlePART(User* user, const std::list<std::string>& params, const Server& server);
		void handleTOPIC(User* user, const std::list<std::string>& params, const Server& server);
		void handleINVITE(User* user, const std::list<std::string>& params, const Server& server);

		// Dominique: codes have been moved to an enum inside enums.hpp (is included by Commands.hpp now)
		// // IRC Numeric Reply Codes
		// static const int RPL_WELCOME = 001;
		// static const int RPL_YOURHOST = 002;
		// static const int RPL_CREATED = 003;
		// static const int RPL_MYINFO = 004;
		// static const int RPL_CHANNELMODEIS = 324;
		// static const int RPL_NOTOPIC = 331;
		// static const int RPL_TOPIC = 332;
		// static const int RPL_INVITING = 341;
		// static const int RPL_NAMREPLY = 353;
		// static const int RPL_ENDOFNAMES = 366;

		// static const int ERR_NOSUCHNICK = 401;
		// static const int ERR_NOSUCHCHANNEL = 403;
		// static const int ERR_TOOMANYCHANNELS = 405;
		// static const int ERR_NORECIPIENT = 411;
		// static const int ERR_NOTEXTTOSEND = 412;
		// static const int ERR_CANNOTSENDTOCHAN = 404;
		// static const int ERR_NONICKNAMEGIVEN = 431;
		// static const int ERR_ERRONEUSNICKNAME = 432;
		// static const int ERR_NICKNAMEINUSE = 433;
		// static const int ERR_USERNOTINCHANNEL = 441;
		// static const int ERR_NOTONCHANNEL = 442;
		// static const int ERR_USERONCHANNEL = 443;
		// static const int ERR_NOTREGISTERED = 451;
		// static const int ERR_NEEDMOREPARAMS = 461;
		// static const int ERR_ALREADYREGISTRED = 462;
		// static const int ERR_PASSWDMISMATCH = 464;
		// static const int ERR_KEYSET = 467;
		// static const int ERR_CHANNELISFULL = 471;
		// static const int ERR_UNKNOWNMODE = 472;
		// static const int ERR_INVITEONLYCHAN = 473;
		// static const int ERR_BADCHANNELKEY = 475;
		// static const int ERROR_CHANOPRIVSNEEDED = 482;
};

#endif
