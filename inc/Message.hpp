/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmitsuya <tmitsuya@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/06 09:29:24 by tmitsuya          #+#    #+#             */
/*   Updated: 2025/11/07 14:23:14 by tmitsuya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MESSAGE_HPP
# define MESSAGE_HPP

# include <iostream>
# include <list>
# include <sstream>
# include <string>
# include <vector>
# include "ChopCommands.hpp"

# define MINIMUM_PARAMS_PASS 1
# define MINIMUM_PARAMS_NICK 0
# define MINIMUM_PARAMS_USER 4
# define MINIMUM_PARAMS_JOIN 1
# define MINIMUM_PARAMS_MSG 0
# define MINIMUM_PARAMS_KICK 2
# define MINIMUM_PARAMS_INVITE 2
# define MINIMUM_PARAMS_TOPIC 1
# define MINIMUM_PARAMS_MODE 1
# define MINIMUM_PARAMS_WHOIS 0

class Server;

typedef enum e_commands
{
	e_pass,
	e_nick,
	e_user,
	e_join,
	e_msg,
	e_kick,
	e_invite,
	e_topic,
	e_mode,
	e_whois,
	e_unkown,
}	t_cmds;

class Message
{

private:

	std::string				m_command{};
	std::list<std::string>	m_params{};
	int						m_nparams{};
	int						m_need_nparams{};
	int						m_cmd_type{};

	static inline const std::vector<std::string>	k_commands ={
		"PASS", "NICK", "USER", "JOIN", "PRIVMSG", "KICK", "INVITE", 
		"TOPIC", "MODE", "WHOIS",
	};

	void	extractCommand(std::istringstream &message);
	void	setCommandType();
	void	setNeedNumParams();
	void	extractParams(std::istringstream &message);

public:

	Message(const std::string &message);
	Message(const Message &other);
	Message	&operator=(const Message &other);
	~Message();

	void	print() const;
	void	operateCommand(Server &server, User &uer);

	const std::string				&getCommandName() const;
	const std::list<std::string>	&getParams() const;
	const int						&getNumParams() const;
	const int						&getNeedNumParas() const;
	const int						&getCmdType() const;

};

#endif
