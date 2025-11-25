/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BotResponseHandler.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 17:32:25 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/25 17:42:52 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BotResponseHandler.hpp"

/* ===================== Bot Response ===================== */

void	BotResponseHandler::sendGrid( int bot_socket_fd, const std::string& username, const std::string& channel, const std::string& header, const Grid& grid )
{
	std::string 		gridMsg { grid.getGridMsg() };
	std::istringstream	iss { gridMsg };
	std::string			line {};

	sendResponse(bot_socket_fd, username, channel, "");
	sendResponse(bot_socket_fd, username, channel, header);
	sendResponse(bot_socket_fd, username, channel, "");
	while ( std::getline(iss, line) )
		sendResponse(bot_socket_fd, username, channel, line);
	sendResponse(bot_socket_fd, username, channel, "");
}

void	BotResponseHandler::sendResponse( int bot_socket_fd, const std::string& username, const std::string& channel, const std::string& msg )
{
	std::string	prefix {};
	std::string	response {};

	if (channel.length())
		prefix = "PRIVMSG " +  channel + " :";
	else
		prefix = "PRIVMSG " +  username + " :";

	response = prefix + msg + "\r\n";

	send(bot_socket_fd, response.data(), response.length(), 0);
}
