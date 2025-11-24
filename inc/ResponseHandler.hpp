/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ResponseHandler.hpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/13 13:47:33 by spyun         #+#    #+#                 */
/*   Updated: 2025/11/24 14:09:24 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSEHANDLER_HPP
#define RESPONSEHANDLER_HPP

#include <string>
#include <sstream>
#include <iomanip>
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <cerrno>
#include "User.hpp"
#include "Channel.hpp"

class ResponseHandler
{
	private:
		ResponseHandler() = delete;
		ResponseHandler(const ResponseHandler&) = delete;
		ResponseHandler& operator=(const ResponseHandler&) = delete;
		~ResponseHandler() = delete;

		static void sendTopicMessage(User* user, Channel* channel);
		static void sendNameMessage(User* user, Channel* channel);

	public:
		static void sendResponse(int fd, const std::string& message);
		static void sendNumericReply(int fd, int code, const std::string& message);
		static void sendNumericReply(int fd, int code, const std::string& nickname, const std::string& message);
		static void sendError(int fd, const std::string& command, const std::string& message);
		static void sendAuthenticationError(int fd, const std::string& command);
		static void sendWelcome(User* user);
		static void sendJoinMessages(User* user, Channel* channel);
};

#endif
