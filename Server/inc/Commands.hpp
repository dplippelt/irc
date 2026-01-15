/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 11:10:22 by spyun             #+#    #+#             */
/*   Updated: 2026/01/15 17:58:26 by dlippelt         ###   ########.fr       */
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
#include "Validator.hpp"
#include "Server.hpp"
#include "IrcNumericCodes.hpp"

class Server;
class Message;

class Command
{
	private:
		Server&							m_server;
		ResponseHandler					m_responseHandler;
		Validator						m_validator;
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
		void handleMODE();

		CommandType						getCmdType() const;
		void							informUsersOfNickChange(const std::string& oldPrefix, const std::string& newNick);
		const std::string				getKickReason() const;
		void							sendKickResponse(Channel* channel, const std::string& channelName, const std::string& targetNick);
		const std::vector<std::string>	getKeyVector() const;
		const std::string				getPartReason() const;
		const std::string				getPartMessage(const std::string& currentChannel) const;
		void							sendPartResponse(Channel* channel, const std::string& currentChannel);
		const std::string				getNewTopic() const;
		const std::string				getQuitReason( const std::vector<std::string>& params ) const;
		const User*						getTargetUser(const std::string& targetNick) const;
		void							sendPrivMsgToChannel(const std::string& target, const std::string& message);
		void							sendPrivMsgToUser(const std::string& target, const std::string& message);
		Channel*						getOrCreateChannel(const std::string& channelName, std::map<std::string, Channel*>& channels);
		const std::vector<std::string>	getChannelVector() const;
		void							removeEmptyChannel(Channel* channel, const std::string& channelName);
		std::string						removeLeadingColon(const std::string& str) const;

		static inline const std::string k_mode_available{ "itkol" };
		static inline const std::string k_modes_without_param{ "it" };
		static inline const std::string k_modes_with_param{ "kol" };

		typedef struct s_modes
		{
			char		sign;
			char		mode;
			std::string	param;
		}	t_mode_elems;


		// handleMODE() Utilities
		int			createSignModePairs(std::vector<t_mode_elems> &mode_param_pairs, const std::string &base);
		void		assignParamsToModes(std::vector<t_mode_elems> &mode_param_pairs);
		int			checkModeparamPairValidation(const std::vector<t_mode_elems> &mode_param_pairs);
		int			changeChannelMode(Channel* channel, const t_mode_elems &mode_param_pairs);
		int			changeChannelModeInvite(Channel* channel, const t_mode_elems &mode_param_pairs);
		int			changeChannelModeTopic(Channel* channel, const t_mode_elems &mode_param_pairs);
		int			changeChannelModeKey(Channel* channel, const t_mode_elems &mode_param_pairs);
		int			changeChannelModePrivilege(Channel* channel, const t_mode_elems &mode_param_pairs);
		int			changeChannelModeLimit(Channel* channel, const t_mode_elems &mode_param_pairs);
		std::string	generateModeResponse(Channel* channel, const std::vector<t_mode_elems> &mode_param_pairs);

		void	formatModeParamToResponse(Channel* channel, std::vector<t_mode_elems> &mode_param_pairs);
		void	formatParamForModeKey(Channel* channel, std::vector<t_mode_elems> &mode_param_pairs);
		void	formatParamForModeLimit(Channel* channel, std::vector<t_mode_elems> &mode_param_pairs);

	public:
		Command() = delete;
		Command(Server& server, User* user, Message& msg);
		Command(const Command&) = delete;
		Command& operator=(const Command&) = delete;
		~Command();

		void executeCommand();
};

#endif
