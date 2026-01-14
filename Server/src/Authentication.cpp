/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Authentication.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/13 15:17:23 by spyun             #+#    #+#             */
/*   Updated: 2025/12/18 17:10:38 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Authentication.hpp"
#include <iostream>

// ==================== Registration Check ====================

void Authentication::checkRegistration(User* user, ResponseHandler& responseHandler)
{
	if (user->hasProvidedPassword() && user->hasNickname() && user->hasUsername() && !user->isRegistered())
	{
		user->setRegistered(true);
		responseHandler.sendWelcome(user);

		// Send empty user mode change to client so that status bar in irssi
		// is always updated with the latest nickname if the nickname during
		// registration changes from the initial nickname (usually the username)
		std::string userModeChangeMsg { user->getPrefix() + " MODE " + user->getNickname() };
		responseHandler.sendResponse(user->getFd(), userModeChangeMsg);
	}
}

// ==================== Command Permission Checks ====================

bool Authentication::canExecuteCommand(User* user, const std::string& command)
{
	if (command == "PASS")
		return true;
	if (command == "NICK" || command == "USER")
	{
		if (!user->hasProvidedPassword())
			return false;
		return true;
	}
	if (!user->isRegistered())
		return false;
	return true;
}

// ==================== Authentication Status ====================

bool Authentication::isAuthenticated(User* user)
{
	return user->hasProvidedPassword();
}

bool Authentication::isRegistered(User* user)
{
	return user->isRegistered();
}

// ==================== Error Handling ====================

void Authentication::sendAuthenticationError(int fd, const std::string& command, ResponseHandler& responseHandler)
{
	if (command == "NICK" || command == "USER")
		responseHandler.sendNumericReply(fd, ERR_NOTREGISTERED, ":You must provide a password first (PASS command)");
	else
		responseHandler.sendNumericReply(fd, ERR_NOTREGISTERED, ":You have not registered");
}

// ==================== Password Validation ====================

bool Authentication::validatePassword(const std::string& providedPassword,
									   const std::string& serverPassword)
{
	return providedPassword == serverPassword;
}
