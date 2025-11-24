/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Commands.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/28 11:10:22 by spyun         #+#    #+#                 */
/*   Updated: 2025/11/24 10:34:21 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>
#include <list>
#include <map>
#include <sstream>
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <iomanip>
#include "User.hpp"
#include "Channel.hpp"
#include "ResponseHandler.hpp"
#include "Authentication.hpp"
#include "ValidationHelper.hpp"

class Commands
{
	private:
		Commands() = delete;
		Commands(const Commands&) = delete;
		Commands& operator=(const Commands&) = delete;
		~Commands() = delete;

		static Channel* getOrCreateChannel(const std::string& channelName, std::map<std::string, Channel*>& channels);

	public:
		static void executeCommand(User* user, const std::string& command,
							const std::vector<std::string>& params,
							std::map<int, User*>& users,
							std::map<std::string, Channel*>& channels,
							const std::string& serverPassword);

		static void handlePASS(User* user, const std::list<std::string>& params, const std::string& serverPassword);
		static void handleNICK(User* user, const std::list<std::string>& params, const std::map<int, User*>& users);
		static void handleUSER(User* user, const std::list<std::string>& params);
		static void handleJOIN(User* user, const std::list<std::string>& params, std::map<std::string, Channel*>& channels);
		static void handlePRIVMSG(User* user, const std::list<std::string>& params, const std::map<int, User*>& users, std::map<std::string, Channel*>& channels);
		static void handleKICK(User* user, const std::list<std::string>& params, const std::map<int, User*>& users, std::map<std::string, Channel*>& channels);
		static void handlePART(User* user, const std::list<std::string>& params, std::map<std::string, Channel*>& channels);
		static void handleTOPIC(User* user, const std::list<std::string>& params, std::map<std::string, Channel*>& channels);
		static void handleINVITE(User* user, const std::list<std::string>& params, const std::map<int, User*>& users, std::map<std::string, Channel*>& channels);
};

#endif
