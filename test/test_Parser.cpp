/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_Parser.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmitsuya <tmitsuya@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 15:13:38 by tmitsuya          #+#    #+#             */
/*   Updated: 2025/10/30 18:28:16 by tmitsuya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"
#include <vector>
// c++ -Wall -Wextra -Werror -std=c++17 -I./inc test/test_Parser.cpp src/Server/Parser.cpp -o test_Parser

int	main()
{
	Parser	parser{};

	std::vector<std::string>	inputs = {
		"KICK #chanell :username :me :some reason\r\n",		// ':' in not the very last parameter (e.g. :user)
		"KICK #chanell username :me :some reason\r\n", 		// ':' in the last trailing comment 	
		":prefix PASS   parameter\r\nNICK  	nickname\r\nJOIN   chanell :test    channel\r\n", // multiple messages at a time
		":prefix JOIN   channel\r\nNICK  	",				// without ending with \r\n
		"nickname\r\nJOIN   chanell :test    channel",
		"\r\n:prefix PASS  \r\n",							// insaficient params 
		"nocommand test \r\n",								// incorrect command
	};

	int	n{ 1 };
	for ( auto it{ inputs.begin() }; it != inputs.end(); ++it)
	{
		std::cout << "<  TEST " << n << "  > \n";
		std::cout << "raw input: \n" << *it << '\n';
		parser.loadInput(*it);
		parser.parse();
		parser.print();
		std::cout << '\n';
		++n;
	}

	// std::istringstream	line{ inputs.back() };
	// std::string			elem{};
	// while(getline(line, elem))
	// {
	// 	std::cout << elem << '\n';
	// 	std::cout << std::boolalpha;
	// 	std::cout << "line.eof(): " << line.eof() << '\n';
	// }

	return 0;

}

