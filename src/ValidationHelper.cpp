/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ValidationHelper.cpp                               :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/13 15:47:35 by spyun         #+#    #+#                 */
/*   Updated: 2025/11/24 09:50:36 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "ValidationHelper.hpp"

// ==================== Nickname Validation ====================

bool ValidationHelper::isValidNickname(const std::string& nick)
{
	if (nick.empty() || nick.length() > 20)
		return false;
	if (!std::isalpha(static_cast<unsigned char>(nick[0])))
		return false;
	for (size_t i = 1; i < nick.length(); ++i)
	{
		char c = nick[i];
		if (!std::isalnum(static_cast<unsigned char>(c))
			&& c != '-' && c != '[' && c != ']' && c != '\\'
			&& c != '`' && c != '^' && c != '{' && c != '}'
			&& c != '_')
		{
			return false;
		}
	}
	return true;
}

bool ValidationHelper::isNicknameInUse(const std::string& nick, const std::map<int, User*>& users)
{
	for (std::map<int, User*>::const_iterator it = users.begin(); it != users.end(); ++it)
	{
		if (it->second->getNickname() == nick)
			return true;
	}
	return false;
}

bool ValidationHelper::isNicknameAvailable(const std::string& nick, int currentUserFd, const std::map<int, User*>& users)
{
	for (std::map<int, User*>::const_iterator it = users.begin(); it != users.end(); ++it)
	{
		if (it->first == currentUserFd)
			continue;
		if (it->second->getNickname() == nick)
			return false;
	}
	return true;
}

bool ValidationHelper::isValidChannelName(const std::string& channelName)
{
	if (channelName.length() < 2 || channelName.length() > 50)
		return false;
	if (channelName[0] != '#' && channelName[0] != '&')
		return false;
	for (size_t i = 1; i < channelName.length(); ++i)
	{
		char c = channelName[i];
		if (std::isspace(static_cast<unsigned char>(c)) || c == ',' || std::iscntrl(static_cast<unsigned char>(c)))
			return false;
	}
	return true;
}

// ==================== Password Validation ====================

bool ValidationHelper::isValidPassword(const std::string& password)
{
	if (password.empty())
		return false;
	for (size_t i = 0; i < password.length(); ++i)
	{
		char c = password[i];
		if (std::isspace(static_cast<unsigned char>(c)) || std::iscntrl(static_cast<unsigned char>(c)))
			return false;
	}
	return true;
}

// ==================== String Helpers ====================

std::string ValidationHelper::removeLeadingColon(const std::string& str)
{
	if (!str.empty() && str[0] == ':')
		return str.substr(1);
	return str;
}
