/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CTCPHandler.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/12 11:35:34 by spyun         #+#    #+#                 */
/*   Updated: 2025/12/12 15:27:28 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CTCPHANDLER_HPP
#define CTCPHANDLER_HPP

#include <string>
#include <vector>

class CTCPHandler
{
	private:
		CTCPHandler();
		~CTCPHandler();
		CTCPHandler(const CTCPHandler&) = delete;
		CTCPHandler& operator=(const CTCPHandler&) = delete;

	public:
		static bool			isCTCPMessage(const std::string& message);
		static std::string	extractCTCPCommand(const std::string& message);
		static bool			isDCCCommand(const std::string& ctcpCommand);
		static bool			parseDCCSend(const std::string& ctcpCommand, std::string& filename, unsigned long& ip, unsigned int& port, unsigned long& filesize);
		static std::string	ipIntToString(unsigned long ip);
};

#endif
