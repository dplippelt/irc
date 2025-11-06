/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmitsuya <tmitsuya@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 13:41:54 by tmitsuya          #+#    #+#             */
/*   Updated: 2025/10/30 16:29:14 by tmitsuya         ###   ########.fr       */
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
// # define 
// # define 
// # define 
// # define 
// # define 
// # define 
// # define 
// # define 
// # define 

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
	e_unkown,
}	t_cmds;

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

	std::string				m_raw_input{};
	std::list<t_message>	m_messages{};

	static inline const std::vector<std::string>	k_commands ={
		"PASS", "NICK", "USER", "JOIN", "PRIVMSG", "KICK", "INVITE", 
		"TOPIC", "MODE",
	};

	void	partitioning(const std::string &input);
	void	partPrefixAndCommand(std::istringstream &message, t_message &elem);
	void	partParams(std::istringstream &message, t_message &elem);
	void	setCommandType(t_message &elem);
	void	paramsValidation(t_message &elem);

public:

	Parser(const std::string &buffer);
	Parser(const Parser &other);
	const Parser	&operator=(const Parser &other);
	~Parser();

	void	parse();
	void	print() const;
};

#endif
