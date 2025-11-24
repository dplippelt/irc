/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmitsuya <tmitsuya@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 13:41:54 by tmitsuya          #+#    #+#             */
/*   Updated: 2025/11/06 14:39:48 by tmitsuya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
# define PARSER_HPP

# include <iostream>
# include <list>
# include <sstream>
# include <string>
# include "Message.hpp"

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
