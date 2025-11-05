/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmitsuya <tmitsuya@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 15:15:41 by tmitsuya          #+#    #+#             */
/*   Updated: 2025/11/05 16:38:49 by tmitsuya         ###   ########.fr       */
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

const Parser	&Parser::operator=(const Parser &other)
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

void	Parser::setParams(std::istringstream &message, t_message &elem, int nim)
{
	std::string	param{};
	int			nparam{};

	while (message >> param)
	{
		elem.params.push_back(param);
		if (nparam >= nim - 1 && param.front() == ':' && getline(message, param))
			elem.params.back() += param;
		++nparam;
	}
	elem.nparams = nparam;
	if (nparam < nim)
		elem.err_type = ERR_NEEDMOREPARAMS;
}

void	Parser::partParams(std::istringstream &message, t_message &elem)
{
	switch (elem.cmd_type)
	{
	case e_pass:
		setParams(message, elem, MINIMUM_PARAMS_PASS);
		break;
	case e_nick:
		setParams(message, elem, MINIMUM_PARAMS_NICK);
		break;
	case e_user:
		setParams(message, elem, MINIMUM_PARAMS_USER);
		break;
	case e_join:
		setParams(message, elem, MINIMUM_PARAMS_JOIN);
		break;
	case e_msg:
		setParams(message, elem, MINIMUM_PARAMS_MSG);
		break;
	case e_kick:
		setParams(message, elem, MINIMUM_PARAMS_KICK);
		break;
	case e_invite:
		setParams(message, elem, MINIMUM_PARAMS_INVITE);
		break;
	case e_topic:
		setParams(message, elem, MINIMUM_PARAMS_TOPIC);
		break;
	case e_mode:
		setParams(message, elem, MINIMUM_PARAMS_MODE);
		break;
	case e_whois:
		setParams(message, elem, MINIMUM_PARAMS_WHOIS);
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
		// paramsValidation(elem);
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

Parser	&Parser::parse()
{
	std::istringstream 	messages{ m_buffer };
	std::string			message{};

	std::cout << "=== m_buffer at the beginning ===\n";
	std::cout << m_buffer << '\n';
	std::cout << "===\n";
	while (getline(messages, message))
	{
		if (messages.eof())
		{
			m_buffer = message;
			return *this;
		}
		message.pop_back();
		partitioning(message);
	}
	m_buffer.clear();
	return *this;
	// TODO: authentification check (PASS --> USER --> NICK)
	// , only after both USER and NICK have been received from a client does a user become registered.
}

Parser	&Parser::loadInput(const std::string &input)
{
	m_buffer += input;
	return *this;
}

std::list<t_message>	&Parser::getMessages()
{
	return m_messages;
}

/* To debug */
void	Parser::print() const
{
	t_message	elem{};

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
		std::cout << "err_type: " << elem.err_type << '\n';
		std::cout << "==\n";
		++n;
	}
	std::cout << "=== m_buffer at the end ===\n";
	std::cout << m_buffer << '\n';
	std::cout << "===\n";

}
