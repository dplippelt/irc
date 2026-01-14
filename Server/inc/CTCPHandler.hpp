/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CTCPHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/12 11:35:34 by spyun             #+#    #+#             */
/*   Updated: 2026/01/14 11:53:33 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CTCPHANDLER_HPP
#define CTCPHANDLER_HPP

#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <User.hpp>

class User;

class CTCPHandler
{
	private:
		CTCPHandler() = delete;
		~CTCPHandler() = delete;
		CTCPHandler(const CTCPHandler&) = delete;
		CTCPHandler& operator=(const CTCPHandler&) = delete;

		static bool			isCTCPMessage(const std::string& message);
		static std::string	extractCTCPCommand(const std::string& message);
		static bool			isDCCCommand(const std::string& ctcpCommand);
		static bool			parseDCCSend(const std::string& ctcpCommand, std::string& filename, unsigned long& ip, unsigned int& port, unsigned long& filesize);
		static std::string	ipIntToString(unsigned long ip);

	public:
		static void			debugDCC(User* user, const std::string& target, const std::string& message);
};

#endif
