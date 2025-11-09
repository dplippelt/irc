/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmitsuya <tmitsuya@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 11:10:22 by spyun             #+#    #+#             */
/*   Updated: 2025/11/07 19:51:07 by tmitsuya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDS_HPP
#define COMMANDS_HPP

# include <algorithm> // [Takato]: added
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <iomanip>
#include "User.hpp"
#include "Channel.hpp"

class Server;
class Message;

class Commands
{
	private:
		std::map<int, User*>& _users;
		std::map<std::string, Channel*>& _channels;
		std::string _serverPassword;

		static void sendResponse(int fd, const std::string& message);  // [Takato]: made this static function, this functionality being a method in Server class would be better ?
		bool isValidNickname(const std::string& nick) const;
		bool isNicknameInUse(const std::string& nick) const;
		void checkRegistration(User* user);

		void sendWelcome(User* user);
		void sendError(int fd, const std::string& command, const std::string& message);
		static void sendNumericReply(int fd, int code, const std::string& message); // [Takato]: made this static function, this functionality being a method in Server class would be better ?

		bool isValidChannelName(const std::string& channelName) const;
		Channel* getOrCreateChannel(const std::string& channelName);
		void sendJoinMessages(User* user, Channel* channel);

		static inline const size_t		k_max_mode_num{ 3 }; // [Takato]: added for mode operation
		static inline const std::string k_mode_set_toggle{ "it" }; // [Takato]: added for mode operation
		static inline const std::string k_mode_set_param{ "kol" }; // [Takato]: added for mode operation
		static void	modeOperateToggle(char mode, char sign, const Message &message, Server &server);
		static void	modeOperateParam(char mode, char sign, const Message &message, Server &server); // [Takato]: added for mode operation
		static void	modeOperateParamPrivilege(char sign, const Message &message, Server &server);
		static void	channelValidation(const Server &server, User *user, const std::string &channel);
		static void	modesValidation(const std::string &modes);

	public:
		Commands(std::map<int, User*>& users,
				 std::map<std::string, Channel*>& channels,
				 const std::string& password);
		~Commands();

		// Main command dispatcher
		void executeCommand(User* user, const std::string& command,
						    const std::vector<std::string>& params);

		// Authentication commands
		void handlePASS(User* user, const std::vector<std::string>& params);
		void handleNICK(User* user, const std::vector<std::string>& params);
		void handleUSER(User* user, const std::vector<std::string>& params);

		// Channel commands
		void handleJOIN(User* user, const std::vector<std::string>& params);

		// Messaging commands
		void handlePRIVMSG(User* user, const std::vector<std::string>& params);

		// IRC Numeric Reply Codes
		static const int RPL_WELCOME = 001;
		static const int RPL_YOURHOST = 002;
		static const int RPL_CREATED = 003;
		static const int RPL_MYINFO = 004;
		static const int RPL_CHANNELMODEIS = 324;
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
		static const int ERR_KEYSET = 467; // [Takato]: added for mode operation
		static const int ERR_CHANNELISFULL = 471;
		static const int ERR_UNKNOWNMODE = 472;      // [Takato]: added for mode operation
		static const int ERR_INVITEONLYCHAN = 473;
		static const int ERR_BADCHANNELKEY = 475;
		static const int ERR_NOCHANMODES = 477;      // [Takato]: added for mode operation
		static const int ERR_CHANOPRIVSNEEDED = 482; // [Takato]: added for mode operation

		static void	mode(const Message &message, Server &server, User *user); // [Takato]: added for mode operation
		
};

#endif
