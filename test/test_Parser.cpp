/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_Parser.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 15:13:38 by tmitsuya          #+#    #+#             */
/*   Updated: 2025/10/30 16:41:21 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"
#include <vector>
// c++ -Wall -Wextra -Werror -std=c++17 -I./inc test/test_Parser.cpp src/Server/Parser.cpp -o test_Parser

int	main()
{
	std::vector<std::string>	inputs = {
		":prefix PASS   parameter\r\nNICK  	nickname\r\nJOIN   chanell :test    channel\r\n",
	};
	int	n{ 1 };
	for ( auto it{ inputs.begin() }; it != inputs.end(); ++it)
	{
		std::cout << "<  TEST " << n << "  > \n";
		Parser	parser(*it);
		parser.parse();
		parser.print();
		std::cout << '\n';
		++n;
	}

	return 0;

}

