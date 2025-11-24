/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Authentication.hpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/13 15:17:25 by spyun         #+#    #+#                 */
/*   Updated: 2025/11/24 10:32:35 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef AUTHENTICATION_HPP
#define AUTHENTICATION_HPP

#include <string>
#include "User.hpp"
#include "ResponseHandler.hpp"

class Authentication
{
	private:
		Authentication() = delete;
		Authentication(const Authentication&) = delete;
		Authentication& operator=(const Authentication&) = delete;
		~Authentication() = delete;

	public:
		static void checkRegistration(User* user);
		static bool canExecuteCommand(User* user, const std::string& command);
		static bool isAuthenticated(User* user);
		static bool isRegistered(User* user);
		static void sendAuthenticationError(int fd, const std::string& command);
		static bool validatePassword(const std::string& providedPassword, const std::string& serverPassword);
};

#endif
