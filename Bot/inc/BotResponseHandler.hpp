/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BotResponseHandler.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 17:27:31 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/25 17:44:41 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include "Grid.hpp"

class BotResponseHandler
{
	public:
		BotResponseHandler() = delete;

		static void	sendGrid( int bot_socket_fd, const std::string& username, const std::string& channel, const std::string& header, const Grid& grid );
		static void	sendResponse( int bot_socket_fd, const std::string& username, const std::string& channel, const std::string& msg );
};
