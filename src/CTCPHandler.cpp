/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CTCPHandler.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/12 11:35:32 by spyun         #+#    #+#                 */
/*   Updated: 2025/12/12 13:55:14 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "CTCPHandler.hpp"
#include <sstream>

CTCPHandler::CTCPHandler() {}

CTCPHandler::~CTCPHandler() {}

bool CTCPHandler::isCTCPMessage(const std::string& message)
{
	return (message.size() >= 2 && message.front() == '\x01' && message.back() == '\x01');
}

std::string CTCPHandler::extractCTCPCommand(const std::string& message)
{
	if (!isCTCPMessage(message))
		return "";

	return message.substr(1, message.size() - 2);
}

bool CPTCPHandler::isDCCCommand(const std::string& ctcpCommand)
{
	return (ctcpCommand.length() >= 4 && ctcpCommand.substr(0, 4) == "DCC ");
}

bool CTCPHandler::parseDCCSend(const std::string& ctcpCommand, std::string& filename, unsigned long& ip, unsigned int& port, unsigned long& filesize)
{
	std::istringstream iss(ctcpCommand);
	std::string dcc, command;

	iss >> dcc >> command;
	if (dcc != "DCC" || command != "SEND")
		return false;

	iss >> filename >> ip >> port >> filesize;

	if (filename.empty() || ip == 0 || port == 0)
		return false;

	return true;
}

std::string CTCPHandler::ipIntToString(unsigned long ip)
{
	std::ostringstream oss;
	oss << ((ip >> 24) & 0xFF) << "."
		<< ((ip >> 16) & 0xFF) << "."
		<< ((ip >> 8) & 0xFF) << "."
		<< (ip & 0xFF);
	return oss.str();
}
