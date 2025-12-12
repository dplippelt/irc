/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Authentication.hpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/13 15:17:25 by spyun         #+#    #+#                 */
/*   Updated: 2025/12/11 16:07:32 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef AUTHENTICATION_HPP
#define AUTHENTICATION_HPP

#include <string>
#include "User.hpp"
#include "ResponseHandler.hpp"
#include "IrcNumericCodes.hpp"

class ResponseHandler;

class Authentication
{
	private:
		Authentication() = delete;
		Authentication(const Authentication&) = delete;
		Authentication& operator=(const Authentication&) = delete;
		~Authentication() = delete;

	public:
		static void checkRegistration(User* user, ResponseHandler& responseHandler);
		static bool canExecuteCommand(User* user, const std::string& command);
		static bool isAuthenticated(User* user);
		static bool isRegistered(User* user);
		static void sendAuthenticationError(int fd, const std::string& command, ResponseHandler& responseHandler);
		static bool validatePassword(const std::string& providedPassword, const std::string& serverPassword);
};

#endif
