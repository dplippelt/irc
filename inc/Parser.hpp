/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmitsuya <tmitsuya@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 13:41:54 by tmitsuya          #+#    #+#             */
/*   Updated: 2025/11/06 11:04:00 by tmitsuya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
# define PARSER_HPP

# include <iostream>
# include <list>
# include <sstream>
# include <string>
# include "Message.hpp"

// To move command action related class
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

// To rename this class to something like 'MessageList'
class Parser
{

private:

	std::string			m_buffer{};
	std::list<Message>	m_messages{};

	void	parse();

public:

	Parser();
	Parser(const Parser &other);
	Parser	&operator=(const Parser &other);
	~Parser();

	Parser				&load(const std::string &input);
	std::list<Message>	&getMessages();
	void				print() const;
};

#endif
