/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ValidationHelper.hpp                               :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/13 13:47:33 by spyun         #+#    #+#                 */
/*   Updated: 2025/12/12 14:00:33 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef VALIDATIONHELPER_HPP
#define VALIDATIONHELPER_HPP

#include <string>
#include <map>
#include <cctype>
#include "User.hpp"

class ValidationHelper
{
	private:
		ValidationHelper() = delete;
		ValidationHelper(const ValidationHelper&) = delete;
		ValidationHelper& operator=(const ValidationHelper&) = delete;
		~ValidationHelper() = delete;

	public:
		static bool			isValidNickname(const std::string& nick);
		static bool			isNicknameInUse(const std::string& nick, const std::map<int, User*>& users);
		static bool			isValidChannelName(const std::string& channelName);
		static bool			isValidPassword(const std::string& password);
		static std::string	removeLeadingColon(const std::string& str);
};

#endif
