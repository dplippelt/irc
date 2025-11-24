/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validation.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/13 15:41:29 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/17 15:32:01 by dlippelt         ###   ########.fr       */
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
		Validation() = delete;

		static void			validatePASS( User* user, const std::list<std::string>& params );
		static std::string	validateNICK( User* user, const std::list<std::string>& params, const Server& server );
		static void			validateUSER( User* user, const std::list<std::string>& params );
		static void			validateJOIN( User* user, const std::list<std::string>& params );
		static void			validatePRIVMSG( User* user, const std::list<std::string>& params );
		static void			validateKICK( User* user, const std::list<std::string>& params, std::string& targetNick, std::string& channelName, std::string& reason );
		static void			validatePART( User* user, const std::list<std::string>& params );
		static std::string	validateTOPIC( User* user, const std::list<std::string>& params );
		static void			validateINVITE( User* user, const std::list<std::string>& params, std::string& targetNick, std::string& channelName );


		static void			validateCanJoin( User* user, Channel* channel, std::string& channelKey );
		static Channel*		validateCanSendMsg( User* user, const std::string& target, const Server& server );
		static Channel*		validateCanKick( User* user, const std::string& channelName, const Server& server );
		static User*		validateCanKickTarget( User* user, Channel* channel, const std::string& targetNick, const Server& server );
		static Channel*		validateCanPart( User* user, const std::string& currentChannel, const Server& server );
		static Channel*		validateCanChangeTopic( User* user, const std::string& channelName, const Server& server );
		static Channel*		validateCanInvite( User* user, const std::string& channelName, const Server& server );
		static User*		validateCanInviteTarget( User* user, Channel* channel, const std::string& channelName, const std::string& targetNick, const Server& server );

	private:
		static bool isValidNickname( const std::string& nick );
		static bool isNicknameInUse( const std::string& nick, const Server& server );
};
