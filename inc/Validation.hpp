/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validation.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/13 15:41:29 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/13 17:48:35 by dlippelt         ###   ########.fr       */
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

		static void			validatePASS(User* user, const std::list<std::string>& params);
		static std::string	validateNICK(User* user, const std::list<std::string>& params, const Server& server);
		static void			validateUSER(User* user, const std::list<std::string>& params);
		static void			validateJOIN(User* user, const std::list<std::string>& params);
		static void			validatePRIVMSG(User* user, const std::list<std::string>& params);
		static void			validateKICK(User* user, const std::list<std::string>& params);


		static void			validateCanJoin(User* user, Channel* channel, std::string& channelKey);
		static Channel*		validateCanSendMsg(User* user, const std::string& target, const Server& server);
		static Channel*		validateCanKick(User* user, const std::string& channelName, const Server& server);
		static User*		validateCanKickTarget(User* user, Channel* channel, const std::string& targetNick, const Server& server);

	private:
		static bool isValidNickname(const std::string& nick);
		static bool isNicknameInUse(const std::string& nick, const Server& server);
};
