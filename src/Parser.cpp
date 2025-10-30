/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 15:15:41 by tmitsuya          #+#    #+#             */
/*   Updated: 2025/10/30 16:39:33 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"

/* ==================== Constructors & Destructor ==================== */

Parser::Parser(const std::string &buffer)
	: m_raw_input{ buffer }
{
	// TODO: parse to the point where the last delimiter "\r\n" in multiple messages is found
	// if not found, throw an error
}

Parser::Parser(const Parser &other)
{
	*this = other;
}

const Parser	&Parser::operator=(const Parser &other)
{
	if (this == &other)
		return *this;
	m_raw_input = other.m_raw_input;
	m_messages = other.m_messages;
	return *this;
}

Parser::~Parser()
{
}

void	Parser::partPrefixAndCommand(std::istringstream &message, t_message &elem)
{
	std::string	component{};

	if (!message)
		return ;
	message >> component;
	if (component.front() == ':')
	{
		elem.prefix = component;
		if (!message)
			return ;
		message >> component;
		elem.command = component;
	}
	else
		elem.command = component;
}

void	Parser::setCommandType(t_message &elem)
{
	int	index{};

	for (auto it{ k_commands.begin() }; it != k_commands.end(); ++it)
	{
		if (*it == elem.command)
			break ;
		++index;
	}
	elem.cmd_type = index;
}

void	Parser::partParams(std::istringstream &message, t_message &elem)
{
	std::string	param{};
	int			nparam{};

	while (message >> param)
	{
		elem.params.push_back(param);
		if (param.front() == ':' && getline(message, param))
			elem.params.back() += param;
		++nparam;
	}
	elem.nparams = nparam;
}

void	Parser::paramsValidation(t_message &elem)
{
	switch (elem.cmd_type)
	{
	case e_pass:
		if (elem.nparams < 1)
			elem.err_type = ERR_NEEDMOREPARAMS;
		break;
	case e_user:
		if (elem.nparams < 4)
			elem.err_type = ERR_NEEDMOREPARAMS;
		break;
	case e_join:
		if (elem.nparams < 1)
			elem.err_type = ERR_NEEDMOREPARAMS;
		break;
	case e_kick:
		if (elem.nparams < 2)
			elem.err_type = ERR_NEEDMOREPARAMS;
		break;
	case e_invite:
		if (elem.nparams < 2)
			elem.err_type = ERR_NEEDMOREPARAMS;
		break;
	case e_topic:
		if (elem.nparams < 1)
			elem.err_type = ERR_NEEDMOREPARAMS;
		break;
	case e_mode:
		if (elem.nparams < 1)
			elem.err_type = ERR_NEEDMOREPARAMS;
		break;
	default:
		break;
	}
}

/*
   The protocol messages must be extracted from the contiguous stream of
   octets.  The current solution is to designate two characters, CR and
   LF, as message separators.   Empty  messages  are  silently  ignored,
   which permits  use  of  the  sequence  CR-LF  between  messages
   without extra problems.

   The extracted message is parsed into the components <prefix>,
   <command> and list of parameters matched either by <middle> or
   <trailing> components. */

void	Parser::partitioning(const std::string &input)
{
	std::istringstream	message{ input };
	t_message			elem{};

	partPrefixAndCommand(message, elem);
	setCommandType(elem);
	if (elem.cmd_type != e_unkown)
	{
		partParams(message, elem);
		paramsValidation(elem);
	}
	else
		elem.err_type = ERR_UNKNOWNCOMMAND;
	m_messages.push_back(elem);
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

void	Parser::parse()
{
	std::istringstream 	messages{ m_raw_input };
	std::string			message{};

	while (getline(messages, message))
	{
		message.pop_back();
		partitioning(message);
	}
	// TODO: authentification check (PASS --> USER --> NICK)
	// , only after both USER and NICK have been received from a client does a user become registered.
}

/* To debug */
void	Parser::print() const
{
	t_message	elem{};

	std::cout << "=== raw_input ===\n";
	std::cout << m_raw_input << '\n';
	std::cout << "===\n";
	int	n{};
	for (auto it{m_messages.begin()}; it != m_messages.end(); ++it)
	{
		std::cout << "== message [" << n << "]: ==\n";
		elem = *it;
		std::cout << "prefix: " << elem.prefix << '\n';
		std::cout << "command: " << elem.command << '\n';
		std::cout << "nparams: " << elem.nparams << '\n';
		int	i{};
		for (auto it{ elem.params.begin() }; it != elem.params.end(); ++it)
		{
			std::cout << "params[" << i << "]: " << *it << '\n';
			i++;
		}
		std::cout << "cmd_type: " << elem.cmd_type << '\n';
		std::cout << "==\n";
		++n;
	}
}
