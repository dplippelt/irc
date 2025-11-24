/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmitsuya <tmitsuya@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/06 10:15:12 by tmitsuya          #+#    #+#             */
/*   Updated: 2025/11/11 15:53:26 by tmitsuya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"
#include "Server.hpp"

/* ==================== Constructors & Destructor ==================== */

Message::Message(const std::string &message)
{
	std::istringstream	iss{ message };

	extractCommand(iss);
	setCommandType();
	setNeedNumParams();
	extractParams(iss);
}

Message::Message(const Message &other)
{
	*this = other;
}

Message	&Message::operator=(const Message &other)
{
	if (this == &other)
		return *this;
	m_command = other.m_command;
	m_params = other.m_params;
	m_nparams = other.m_need_nparams;
	m_cmd_type = other.m_cmd_type;
	return *this;
}

Message::~Message()
{
}

/* ==================== Public interfaces ==================== */

void	Message::operateCommand(Server &server, User *user) const
{
	print();
	switch (m_cmd_type)
	{
	// case e_pass:
	// 	break;
	// case e_nick:
	// 	break;
	// case e_user:
	// 	break;
	// case e_join:
	// 	break;
	// case e_msg:
	// 	break;
	// case e_kick:
	// 	break;
	// case e_invite:
	// 	break;
	// case e_topic:
	// 	break;
	case e_mode:
		Commands::mode(*this, server, user);
		break;	
	// case e_whois:
	// 	break;
	default:
		break;
	}
}

/* getters */
const std::string	&Message::getCommandName() const
{
	return m_command;
}

const std::list<std::string>	&Message::getParamsList() const
{
	return m_params;
}

const std::string	&Message::getParam(int index) const
{
	// TODO:
	// if (index > m_params.size())
	// 	throw Exception;
	return *(std::next(m_params.begin(), index));
}

const int	&Message::getNumParams() const
{
	return m_nparams;
}

const int	&Message::getNeedNumParas() const
{
	return m_need_nparams;
}

const int	&Message::getCmdType() const
{
	return m_cmd_type;
}

/* Debug functions */
void	Message::print() const
{
	std::cout << "command: " << m_command << '\n';
	std::cout << "nparams: " << m_nparams << '\n';
	int i{};
	for (auto it{ m_params.begin() }; it != m_params.end(); ++it)
	{
		std::cout << "params[" << i << "]: " << *it << '\n';
		++i;
	}
	std::cout << "need_nparams: " << m_nparams << '\n';
	std::cout << "cmd_type: " << m_cmd_type << '\n';
}

/* ==================== Helpers for Constructors ==================== */

void	Message::extractCommand(std::istringstream &message)
{
	std::string	command{};

	if (!message)
		return ;
	message >> command;
	m_command = command;
}


void	Message::setCommandType()
{
	int	index{};

	for (auto it{ k_commands.begin() }; it != k_commands.end(); ++it)
	{
		if (*it == m_command)
			break ;
		++index;
	}
	m_cmd_type = index;
}

void	Message::setNeedNumParams()
{
	switch (m_cmd_type)
	{
	case e_pass:
		m_need_nparams = MINIMUM_PARAMS_PASS;
		break;
	case e_nick:
		m_need_nparams = MINIMUM_PARAMS_NICK;
		break;
	case e_user:
		m_need_nparams = MINIMUM_PARAMS_USER;
		break;
	case e_join:
		m_need_nparams = MINIMUM_PARAMS_JOIN;
		break;
	case e_msg:
		m_need_nparams = MINIMUM_PARAMS_MSG;
		break;
	case e_kick:
		m_need_nparams = MINIMUM_PARAMS_KICK;
		break;
	case e_invite:
		m_need_nparams = MINIMUM_PARAMS_INVITE;
		break;
	case e_topic:
		m_need_nparams = MINIMUM_PARAMS_TOPIC;
		break;
	case e_mode:
		m_need_nparams = MINIMUM_PARAMS_MODE;
		break;
	case e_whois:
		m_need_nparams = MINIMUM_PARAMS_WHOIS;
		break;
	default:
		break;
	}
}

void	Message::extractParams(std::istringstream &message)
{
	std::string	param{};
	int			nparam{};

	while (message >> param)
	{
		m_params.push_back(param);
		if (nparam >= m_need_nparams - 1 && param.front() == ':' && getline(message, param))
			m_params.back() += param;
		++nparam;
	}
	m_nparams = nparam;
}
