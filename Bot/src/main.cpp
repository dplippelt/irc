/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 10:42:43 by dlippelt          #+#    #+#             */
/*   Updated: 2025/12/15 13:22:46 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bot.hpp"

volatile sig_atomic_t g_quit = 0;

int	main( int ac, char *av[] )
{
	if (ac != 3)
	{
		std::cerr << "Error: incorrect number of parameters!\n";
		std::cerr << "Usage: ./ircbot <server_port> <server password>" << std::endl;
		return (EXIT_FAILURE);
	}

	Bot::setupSigHandler();

	try
	{
		Bot bot { av[1], av[2] };
		while (!g_quit)
			bot.doPoll();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

	return (EXIT_SUCCESS);
}

