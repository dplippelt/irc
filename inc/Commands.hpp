/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Commands.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/28 11:10:22 by spyun         #+#    #+#                 */
/*   Updated: 2025/11/24 14:10:33 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <algorithm>
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
#include "ResponseHandler.hpp"
#include "Authentication.hpp"
#include "ValidationHelper.hpp"
#include "Validation.hpp"
#include "Server.hpp"
#include "IrcNumericCodes.hpp"

class Server;
class Validation;

class Server;
class Message;

class Commands
{
	private:
		Commands() = delete;
		Commands(const Commands&) = delete;
		Commands& operator=(const Commands&) = delete;
		~Commands() = delete;

		static Channel* getOrCreateChannel(const std::string& channelName, std::map<std::string, Channel*>& channels);

		static inline const size_t		k_max_mode_num{ 3 }; // [Takato]: added for mode operation
		static inline const std::string k_mode_set_toggle{ "it" }; // [Takato]: added for mode operation
		static inline const std::string k_mode_set_param{ "kol" }; // [Takato]: added for mode operation
		static void	modeOperateToggle(char mode, char sign, const Message &message, Server &server); // [Takato]: added for mode operation
		static void	modeOperateToggleInvite(char sign, const Message &message, Server &server); // [Takato]: added for mode operation
		static void	modeOperateToggleTopic(char sign, const Message &message, Server &server); // [Takato]: added for mode operation
		static void	modeOperateParam(char mode, char sign, const Message &message, Server &server); // [Takato]: added for mode operation
		static void	modeOperateParamPrivilege(char sign, const Message &message, Server &server); // [Takato]: added for mode operation
		static void	modeOperateParamKey(char sign, const Message &message, Server &server); // [Takato]: added for mode operation
		static void	modeOperateParamLimit(char sign, const Message &message, Server &server); // [Takato]: added for mode operation
		static void	channelValidation(const Server &server, User *user, const std::string &channel); // [Takato]: added for mode operation
		static void	modesValidation(const std::string &modes); // [Takato]: added for mode operation

	public:
		static void executeCommand(User* user, const std::string& command,
							const std::vector<std::string>& params,
							Server& server,
							const std::string& serverPassword);

		static void handlePASS(User* user, const std::list<std::string>& params, const std::string& serverPassword);
		static void handleNICK(User* user, const std::list<std::string>& params, Server& server);
		static void handleUSER(User* user, const std::list<std::string>& params);
		static void handleJOIN(User* user, const std::list<std::string>& params, Server& server);
		static void handlePRIVMSG(User* user, const std::list<std::string>& params, Server& server);
		static void handleKICK(User* user, const std::list<std::string>& params, Server& server);
		static void handlePART(User* user, const std::list<std::string>& params, Server& server);
		static void handleTOPIC(User* user, const std::list<std::string>& params, Server& server);
		static void handleINVITE(User* user, const std::list<std::string>& params, Server& server);
};

#endif
