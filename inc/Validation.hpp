/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Validation.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: dlippelt <dlippelt@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/13 15:41:29 by dlippelt      #+#    #+#                 */
/*   Updated: 2025/11/24 12:49:21 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <exception>
#include <list>
#include <string>
#include "User.hpp"
#include "Commands.hpp"
#include "Server.hpp"

class User;
class Commands;
class Server;

class Validation
{
	public:
		static bool			validatePASS( User* user, const std::list<std::string>& params );
		static bool			validateNICK( User* user, const std::list<std::string>& params, const Server& server, std::string& outNick );
		static bool			validateUSER( User* user, const std::list<std::string>& params );
		static bool			validateJOIN( User* user, const std::list<std::string>& params );
		static bool			validatePRIVMSG( User* user, const std::list<std::string>& params );
		static bool			validateKICK( User* user, const std::list<std::string>& params, std::string& targetNick, std::string& channelName, std::string& reason );
		static bool			validatePART( User* user, const std::list<std::string>& params );
		static bool			validateTOPIC( User* user, const std::list<std::string>& params, std::string& outChannelName );
		static bool			validateINVITE( User* user, const std::list<std::string>& params, std::string& targetNick, std::string& channelName );

		static bool			validateCanJoin( User* user, Channel* channel, std::string& channelKey );
		static Channel*		validateCanSendMsg( User* user, const std::string& target, const Server& server );
		static Channel*		validateCanKick( User* user, const std::string& channelName, const Server& server );
		static User*		validateCanKickTarget( User* user, Channel* channel, const std::string& targetNick, const Server& server );
		static Channel*		validateCanPart( User* user, const std::string& currentChannel, const Server& server );
		static Channel*		validateCanChangeTopic( User* user, const std::string& channelName, const Server& server );
		static Channel*		validateCanInvite( User* user, const std::string& channelName, const Server& server );
		static User*		validateCanInviteTarget( User* user, Channel* channel, const std::string& channelName, const std::string& targetNick, const Server& server );

	private:
		Validation() = delete;
		Validation(const Validation&) = delete;
		Validation& operator=(const Validation&) = delete;
		~Validation() = delete;
};
