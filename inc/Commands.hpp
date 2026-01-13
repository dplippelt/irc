/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 11:10:22 by spyun             #+#    #+#             */
/*   Updated: 2026/01/13 12:27:18 by dlippelt         ###   ########.fr       */
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

class Command
{
	private:
		Server&							m_server;
		ResponseHandler					m_responseHandler;
		User*							m_user;
		const std::string&				m_command;
		const std::vector<std::string>&	m_params;

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

		CommandType getCmdType()
		{
			auto it { k_commands.find(m_command) };

			return ( it != k_commands.end() ? it->second : CMD_UNKNOWN );
		}

		Channel* getOrCreateChannel(const std::string& channelName, std::map<std::string, Channel*>& channels);

		static inline const size_t		k_max_mode_num{ 3 }; // [Takato]: added for mode operation
		static inline const std::string k_mode_set_toggle{ "it" }; // [Takato]: added for mode operation
		static inline const std::string k_mode_set_param{ "kol" }; // [Takato]: added for mode operation

		void	modeOperateToggle(char mode, char sign); // [Takato]: added for mode operation
		void	modeOperateToggleInvite(char sign); // [Takato]: added for mode operation
		void	modeOperateToggleTopic(char sign); // [Takato]: added for mode operation
		void	modeOperateParam(char mode, char sign, int idxOffset); // [Takato]: added for mode operation
		void	modeOperateParamPrivilege(char sign, int idxOffset); // [Takato]: added for mode operation
		void	modeOperateParamKey(char sign, int idxOffset); // [Takato]: added for mode operation
		void	modeOperateParamLimit(char sign, int idxOffset); // [Takato]: added for mode operation

		void handlePASS();
		void handleNICK();
		void handleUSER();
		void handlePING();
		void handleJOIN();
		void handlePRIVMSG();
		void handleKICK();
		void handlePART();
		void handleTOPIC();
		void handleINVITE();
		void handleQUIT();
		void handleWHOIS();
		void handleMODE(); // [Takato]: added for mode operation

		void							informUsersOfNickChange(const std::string& oldPrefix, const std::string& newNick);
		const std::string				getKickReason() const;
		void							sendKickResponse(Channel* channel, const std::string& channelName, const std::string& targetNick);
		const std::vector<std::string>	getChannelVector() const;
		const std::vector<std::string>	getKeyVector() const;
		const std::string				getPartReason() const;
		const std::string				getPartMessage(const std::string& currentChannel) const;
		void							sendPartResponse(Channel* channel, const std::string& currentChannel);

		void							removeEmptyChannel(Channel* channel, const std::string& channelName);

	public:
		Command() = delete;
		Command(Server& server, User* user, Message& msg);
		Command(const Command&) = delete;
		Command& operator=(const Command&) = delete;
		~Command();

		void executeCommand();
};

// try to use struct to make code more readable
typedef struct s_cmd_data
{
	Channel*	channel {};
	User*		user {};
}				t_cmd_data;

#endif
