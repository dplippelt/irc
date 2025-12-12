/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Commands.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: dlippelt <dlippelt@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/28 11:10:22 by spyun         #+#    #+#                 */
/*   Updated: 2025/12/12 09:51:55 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <algorithm>
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
#include "ResponseHandler.hpp"
#include "Authentication.hpp"
#include "ValidationHelper.hpp"
#include "Validation.hpp"
#include "Server.hpp"
#include "IrcNumericCodes.hpp"

class Server;
class Validation;
class Message;

class Commands
{
	private:
		Commands() = delete;
		Commands(const Commands&) = delete;
		Commands& operator=(const Commands&) = delete;
		~Commands() = delete;

		enum CommandType
		{
			CMD_PASS,
			CMD_NICK,
			CMD_USER,
			CMD_PING,
			CMD_JOIN,
			CMD_PRIVMSG,
			CMD_KICK,
			CMD_PART,
			CMD_TOPIC,
			CMD_INVITE,
			CMD_QUIT,
			CMD_WHOIS,
			CMD_MODE,
			CMD_UNKNOWN
		};

		static inline const std::map<std::string, CommandType> k_commands
		{
			{"PASS", CMD_PASS},
			{"NICK", CMD_NICK},
			{"USER", CMD_USER},
			{"PING", CMD_PING},
			{"JOIN", CMD_JOIN},
			{"PRIVMSG", CMD_PRIVMSG},
			{"KICK", CMD_KICK},
			{"PART", CMD_PART},
			{"TOPIC", CMD_TOPIC},
			{"INVITE", CMD_INVITE},
			{"QUIT", CMD_QUIT},
			{"WHOIS", CMD_WHOIS},
			{"MODE", CMD_MODE}
		};

		static CommandType getCmdType( const std::string& command )
		{
			auto it { k_commands.find(command) };

			return ( it != k_commands.end() ? it->second : CMD_UNKNOWN );
		}

		static Channel* getOrCreateChannel(const std::string& channelName, std::map<std::string, Channel*>& channels);

		static inline const size_t		k_max_mode_num{ 3 }; // [Takato]: added for mode operation
		static inline const std::string k_mode_set_toggle{ "it" }; // [Takato]: added for mode operation
		static inline const std::string k_mode_set_param{ "kol" }; // [Takato]: added for mode operation
		static void	modeOperateToggle(char mode, char sign, const std::vector<std::string>& params, Server &server); // [Takato]: added for mode operation
		static void	modeOperateToggleInvite(char sign, const std::vector<std::string>& params, Server &server); // [Takato]: added for mode operation
		static void	modeOperateToggleTopic(char sign, const std::vector<std::string>& params, Server &server); // [Takato]: added for mode operation
		static void	modeOperateParam(char mode, char sign, const std::vector<std::string>& params, Server &server, int idxOffset); // [Takato]: added for mode operation
		static void	modeOperateParamPrivilege(char sign, const std::vector<std::string>& params, Server &server, int idxOffset); // [Takato]: added for mode operation
		static void	modeOperateParamKey(char sign, const std::vector<std::string>& params, Server &server, int idxOffset); // [Takato]: added for mode operation
		static void	modeOperateParamLimit(char sign, const std::vector<std::string>& params, Server &server, int idxOffset); // [Takato]: added for mode operation

	public:
		static void executeCommand(User* user, const std::string& command,
							const std::vector<std::string>& params,
							Server& server,
							ResponseHandler& responseHandler,
							const std::string& serverPassword);

		static void handlePASS(User* user, const std::vector<std::string>& params, ResponseHandler& responseHandler, const std::string& serverPassword);
		static void handleNICK(User* user, const std::vector<std::string>& params, Server& server, ResponseHandler& responseHandler);
		static void handleUSER(User* user, const std::vector<std::string>& params, ResponseHandler& responseHandler);
		static void handlePING(User* user, const std::vector<std::string>& params);
		static void handleJOIN(User* user, const std::vector<std::string>& params, Server& server, ResponseHandler& responseHandler);
		static void handlePRIVMSG(User* user, const std::vector<std::string>& params, Server& server, ResponseHandler& responseHandler);
		static void handleKICK(User* user, const std::vector<std::string>& params, Server& server, ResponseHandler& responseHandler);
		static void handlePART(User* user, const std::vector<std::string>& params, Server& server, ResponseHandler& responseHandler);
		static void handleTOPIC(User* user, const std::vector<std::string>& params, Server& server, ResponseHandler& responseHandler);
		static void handleINVITE(User* user, const std::vector<std::string>& params, Server& server, ResponseHandler& responseHandler);
		static void handleQUIT(User* user, const std::vector<std::string>& params, Server& server, ResponseHandler& responseHandler);
		static void handleWHOIS(User* user, const std::vector<std::string>& params, Server& server, ResponseHandler& responseHandler);
		static void	handleMODE(User *user, const std::vector<std::string>& params, Server &server, ResponseHandler& responseHandler); // [Takato]: added for mode operation
};

#endif
