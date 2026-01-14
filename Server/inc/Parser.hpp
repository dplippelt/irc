/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmitsuya <tmitsuya@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 13:41:54 by tmitsuya          #+#    #+#             */
/*   Updated: 2026/01/14 14:02:24 by tmitsuya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
# define PARSER_HPP

# include <iostream>
# include <list>
# include <sstream>
# include <string>
# include "Message.hpp"

class Parser
{

private:

	std::string			m_buffer{};
	std::list<Message>	m_messages{};

public:

	Parser();
	Parser(const Parser &other);
	Parser	&operator=(const Parser &other);
	~Parser();

	Parser				&parse(const std::string &input);
	std::list<Message>	&getMessages();
	void				print() const;
};

#endif
