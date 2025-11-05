/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Commands.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/28 11:10:22 by spyun         #+#    #+#                 */
/*   Updated: 2025/11/05 09:52:26 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>
#include <vector>
#include <map>
#include "User.hpp"
#include "Channel.hpp"

class Commands
{
	private:
		std::map<int, User*>& _users;                    // Reference to users map
		std::map<std::string, Channel*>& _channels;      // Reference to channels map
		std::string _serverPassword;                     // Server password for PASS command

		void sendResponse(int fd, const std::string& message);
		bool isValidNickname(const std::string& nick) const;
		bool isNicknameInUse(const std::string& nick) const;
		void checkRegistration(User* user);

		void sendWelcome(User* user);
		void sendError(int fd, const std::string& command, const std::string& message);
		void sendNumericReply(int fd, int code, const std::string& message);

	public:
		Commands(std::map<int, User*>& users,
				 std::map<std::string, Channel*>& channels,
				 const std::string& password);
		~Commands();

		// Main command dispatcher
		void executeCommand(User* user, const std::string& command,
						    const std::vector<std::string>& params);

		// Authentication commands
		void handlePASS(User* user, const std::vector<std::string>& params);
		void handleNICK(User* user, const std::vector<std::string>& params);
		void handleUSER(User* user, const std::vector<std::string>& params);

		// IRC Numeric Reply Codes (commonly used ones)
		static const int RPL_WELCOME = 001;
		static const int RPL_YOURHOST = 002;
		static const int RPL_CREATED = 003;
		static const int RPL_MYINFO = 004;

		static const int ERR_NOSUCHNICK = 401;
		static const int ERR_NOSUCHCHANNEL = 403;
		static const int ERR_NONICKNAMEGIVEN = 431;
		static const int ERR_ERRONEUSNICKNAME = 432;
		static const int ERR_NICKNAMEINUSE = 433;
		static const int ERR_NEEDMOREPARAMS = 461;
		static const int ERR_ALREADYREGISTRED = 462;
		static const int ERR_PASSWDMISMATCH = 464;
};

#endif
