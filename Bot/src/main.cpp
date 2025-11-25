/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 10:42:43 by dlippelt          #+#    #+#             */
/*   Updated: 2025/11/20 12:16:01 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bot.hpp"

int	main( int ac, char *av[] )
{
	if (ac != 4)
	{
		std::cerr << "Error: incorrect number of parameters!\n";
		std::cerr << "Usage: ./ircbot <server_address> <server_port> <server password>" << std::endl;
		return (EXIT_FAILURE);
	}

	try
	{
		Bot bot { av[1], av[2], av[3] };
		while (true)
			bot.doPoll();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

	return (EXIT_SUCCESS);
}

