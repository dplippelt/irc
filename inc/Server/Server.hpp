/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 13:04:53 by dlippelt          #+#    #+#             */
/*   Updated: 2025/10/27 13:32:53 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <sys/socket.h>
#include "errno.h"

class Server
{
	public:
		~Server();
		Server() = delete;
		Server( const std::string& port, std::string_view pw );
		Server( const Server& );
		Server& operator=( const Server& );

	private:
		std::string	m_pw {};
		int			m_port {};
		int			m_socket_fd {};

};
