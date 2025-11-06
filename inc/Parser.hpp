/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmitsuya <tmitsuya@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 13:41:54 by tmitsuya          #+#    #+#             */
/*   Updated: 2025/11/05 16:37:28 by tmitsuya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
# define PARSER_HPP

# include <string>
# include <list>
# include <vector>
# include <sstream>
# include <iostream>

# define ERR_UNKNOWNCOMMAND 421
/* 		421     ERR_UNKNOWNCOMMAND
                        "<command> :Unknown command"

                - Returned to a registered client to indicate that the
                  command sent is unknown by the server. */

# define ERR_NONICKNAMEGIVEN 431
# define ERR_ERRONEUSNICKNAME 432
# define ERR_NICKNAMEINUSE 433
# define ERR_NICKCOLLISION 436
# define ERR_NEEDMOREPARAMS 461
/*        461    ERR_NEEDMOREPARAMS
              "<command> :Not enough parameters"

         - Returned by the server by numerous commands to
           indicate to the client that it didn't supply enough
           parameters.
 */
# define ERR_ALREADYREGISTRED 462


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

typedef struct s_message
{
	std::string				prefix;
	std::string				command;
	std::list<std::string>	params;
	int						nparams;
	int						cmd_type;
	int						err_type;
}	t_message;

class Parser
{

private:

	std::string				m_buffer{};
	std::list<t_message>	m_messages{};

	static inline const std::vector<std::string>	k_commands ={
		"PASS", "NICK", "USER", "JOIN", "PRIVMSG", "KICK", "INVITE", 
		"TOPIC", "MODE", "WHOIS",
	};

	void	partitioning(const std::string &input);
	void	partPrefixAndCommand(std::istringstream &message, t_message &elem);
	void	partParams(std::istringstream &message, t_message &elem);
	void	setParams(std::istringstream &message, t_message &elem, int nim);
	void	setCommandType(t_message &elem);
	// void	paramsValidation(t_message &elem);

public:

	Parser();
	Parser(const Parser &other);
	const Parser	&operator=(const Parser &other);
	~Parser();

	Parser	&loadInput(const std::string &input);
	Parser	&parse();
	std::list<t_message>	&getMessages();
	void	print() const;
};

#endif
