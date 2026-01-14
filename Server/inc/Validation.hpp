/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validation.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmitsuya <tmitsuya@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/13 15:41:29 by dlippelt          #+#    #+#             */
/*   Updated: 2026/01/14 14:18:04 by tmitsuya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <exception>
#include <vector>
#include <string>
#include "User.hpp"
#include "Commands.hpp"
#include "Server.hpp"
#include "IrcNumericCodes.hpp"

class User;
class Command;
class Server;
class ResponseHandler;

class Validation
{
	public:
		static bool			validatePASS( User* user, const std::vector<std::string>& params, ResponseHandler& responseHandler );
		static bool			validateNICK( User* user, const std::vector<std::string>& params, const Server& server, std::string& outNick, ResponseHandler& responseHandler );
		static bool			validateUSER( User* user, const std::vector<std::string>& params, ResponseHandler& responseHandler );
		static bool			validateJOIN( User* user, const std::vector<std::string>& params, ResponseHandler& responseHandler );
		static bool			validatePRIVMSG( User* user, const std::vector<std::string>& params, ResponseHandler& responseHandler );
		static bool			validateKICK( User* user, const std::vector<std::string>& params, std::string& targetNick, std::string& channelName, ResponseHandler& responseHandler );
		static bool			validatePART( User* user, const std::vector<std::string>& params, ResponseHandler& responseHandler );
		static bool			validateTOPIC( User* user, const std::vector<std::string>& params, std::string& outChannelName, ResponseHandler& responseHandler );
		static bool			validateINVITE( User* user, const std::vector<std::string>& params, std::string& targetNick, std::string& channelName, ResponseHandler& responseHandler );
		static bool			validateWHOIS( User* user, const std::vector<std::string>& params, std::string& targetNick, ResponseHandler& responseHandler );
		static Channel*		validateMODE( User* user, const std::vector<std::string>& params, const Server& server, std::string& channelName, ResponseHandler& responseHandler );

		static bool			validateCanJoin( User* user, Channel* channel, std::string& channelKey, ResponseHandler& responseHandler );
		static Channel*		validateCanSendMsg( User* user, const std::string& target, const Server& server, ResponseHandler& responseHandler );
		static Channel*		validateCanKick( User* user, const std::string& channelName, const Server& server, ResponseHandler& responseHandler );
		static User*		validateCanKickTarget( User* user, Channel* channel, const std::string& targetNick, const Server& server, ResponseHandler& responseHandler );
		static Channel*		validateCanPart( User* user, const std::string& currentChannel, const Server& server, ResponseHandler& responseHandler );
		static Channel*		validateCanChangeTopic( User* user, const std::string& channelName, const Server& server, ResponseHandler& responseHandler );
		static Channel*		validateCanInvite( User* user, const std::string& channelName, const Server& server, ResponseHandler& responseHandler );
		static User*		validateCanInviteTarget( User* user, Channel* channel, const std::string& channelName, const std::string& targetNick, const Server& server, ResponseHandler& responseHandler );
		static bool			validateCanChangeModes( User* user, Channel* channel, const std::string& channelName, ResponseHandler& responseHandler );
		static bool			validateModeCharacter( User* user, char mode, const std::string& availableModes, ResponseHandler& responseHandler );
		static void			handleModeOperationError( User* user, const std::string& channelName, IrcNumericCodes error_code, ResponseHandler& responseHandler );

	private:
		Validation() = delete;
		Validation(const Validation&) = delete;
		Validation& operator=(const Validation&) = delete;
		~Validation() = delete;
};
