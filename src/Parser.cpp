/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 15:15:41 by tmitsuya          #+#    #+#             */
/*   Updated: 2025/11/25 12:15:44 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"

/* ==================== Constructors & Destructor ==================== */

Parser::Parser()
	: m_buffer{}, m_messages{}
{
}

Parser::Parser(const Parser &other)
{
	*this = other;
}

Parser	&Parser::operator=(const Parser &other)
{
	if (this == &other)
		return *this;
	m_buffer = other.m_buffer;
	m_messages = other.m_messages;
	return *this;
}

Parser::~Parser()
{
}

/* ==================== Public interfaces ==================== */

std::list<Message>	&Parser::getMessages()
{
	return m_messages;
}

/* Debug functions */

void	Parser::print() const
{
	int	n{};
	for (auto it{m_messages.begin()}; it != m_messages.end(); ++it)
	{
		std::cout << "== message [" << n << "]: ==\n";
		(*it).print();
		std::cout << "==\n";
		++n;
	}
	std::cout << "=== m_buffer at the end ===\n";
	std::cout << m_buffer << '\n';
	std::cout << "===\n";
}

/* ==================== Private helpers ==================== */

/*
   The protocol messages must be extracted from the contiguous stream of
   octets.  The current solution is to designate two characters, CR and
   LF, as message separators.   Empty  messages  are  silently  ignored,
   which permits  use  of  the  sequence  CR-LF  between  messages
   without extra problems.

   The extracted message is parsed into the components <prefix>,
   <command> and list of parameters matched either by <middle> or
   <trailing> components.
*/

Parser	&Parser::parse(const std::string &input)
{
	m_buffer += input;
	std::cout << "=== m_buffer at the beginning ===\n";
	std::cout << m_buffer << "\n===\n";

	std::istringstream 	messages{ m_buffer };
	std::string			message{};

	while (getline(messages, message))
	{
		if (messages.eof())
		{
			m_buffer = message;
			return *this;
		}
		message.pop_back();
		m_messages.emplace_back(Message{message});
	}
	m_buffer.clear();
	return *this;
}

/*
   IRC messages are always lines of characters terminated with a CR-LF
   (Carriage Return - Line Feed) pair, and these messages shall not
   exceed 512 characters in length, counting all characters including
   the trailing CR-LF. Thus, there are 510 characters maximum allowed
   for the command and its parameters.  There is no provision for
   continuation message lines.  See section 7 for more details about
   current implementations. */
/*
   The server to which a client is connected is required to parse the
   complete message, returning any appropriate errors.  If the server
   encounters a fatal error while parsing a message, an error must be
   sent back to the client and the parsing terminated.  A fatal error
   may be considered to be incorrect command, a destination which is
   otherwise unknown to the server (server, nick or channel names fit
   this category), not enough parameters or incorrect privileges.
   If a full set of parameters is presented, then each must be checked
   for validity and appropriate responses sent back to the client.  In
   the case of messages which use parameter lists using the comma as an
   item separator, a reply must be sent for each item.
*/

