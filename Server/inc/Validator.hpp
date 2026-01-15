/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validator.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 12:55:42 by dlippelt          #+#    #+#             */
/*   Updated: 2026/01/15 17:44:19 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <exception>
#include <vector>
#include <string>
#include "User.hpp"
#include "Server.hpp"
#include "IrcNumericCodes.hpp"

class User;
class Server;
class ResponseHandler;

class Validator
{
	public:
		~Validator();
		Validator() = delete;
		Validator(Server& server, User* user, const std::vector<std::string>& params);
		Validator(const Validator&) = delete;
		Validator& operator=(const Validator&) = delete;

		bool		validatePASS();
		bool		validateNICK( std::string& outNick );
		bool		validateUSER();
		bool		validateJOIN();
		bool		validatePRIVMSG();
		bool		validateKICK( std::string& targetNick, std::string& channelName );
		bool		validatePART();
		bool		validateTOPIC( std::string& outChannelName );
		bool		validateINVITE( std::string& targetNick, std::string& channelName );
		bool		validateWHOIS( std::string& targetNick );
		Channel*	validateMODE( std::string& channelName );

		bool		validateCanJoin( Channel* channel, std::string& channelKey );
		Channel*	validateCanSendMsg( const std::string& target );
		Channel*	validateCanKick( const std::string& channelName );
		User*		validateCanKickTarget( Channel* channel, const std::string& targetNick );
		Channel*	validateCanPart( const std::string& currentChannel );
		Channel*	validateCanChangeTopic( const std::string& channelName );
		Channel*	validateCanInvite( const std::string& channelName );
		User*		validateCanInviteTarget( Channel* channel, const std::string& channelName, const std::string& targetNick );
		bool		validateCanChangeModes( Channel* channel, const std::string& channelName );
		void		handleModeOperationError( const std::string& channelName, IrcNumericCodes error_code );

		bool		isValidNickname(const std::string& nick);
		bool		isNicknameInUse(const std::string& nick, const std::map<int, User*>& users);
		bool		isValidChannelName(const std::string& channelName);
		bool		isValidPassword(const std::string& password);

	private:
		Server&							m_server;
		ResponseHandler					m_responseHandler;
		User*							m_user;
		const std::vector<std::string>&	m_params;
};
