/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseHandler.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/13 13:47:33 by spyun             #+#    #+#             */
/*   Updated: 2026/01/13 13:00:23 by dlippelt         ###   ########.fr       */
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
#include "IrcNumericCodes.hpp"

class Server;

class ResponseHandler
{
	private:
		Server& m_server;

		void sendNameMessage(User* user, Channel* channel);

	public:
		ResponseHandler(Server& server);
		ResponseHandler(const ResponseHandler&) = delete;
		ResponseHandler& operator=(const ResponseHandler&) = delete;
		~ResponseHandler();

		void sendResponse(int fd, const std::string& message);
		void sendNumericReply(int fd, int code, const std::string& message);
		void sendNumericReply(int fd, int code, const std::string& nickname, const std::string& message);
		void sendError(int fd, const std::string& command, const std::string& message);
		void sendAuthenticationError(int fd, const std::string& command);
		void sendWelcome(User* user);
		void sendJoinMessages(User* user, Channel* channel);
		void sendTopicMessage(User* user, Channel* channel);
};

#endif
