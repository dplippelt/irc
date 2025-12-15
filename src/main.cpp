/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 12:36:05 by dlippelt          #+#    #+#             */
/*   Updated: 2025/12/15 13:02:31 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

volatile sig_atomic_t g_quit = 0;

int	main( int ac, char *av[] )
{
	if (ac != 3)
	{
		std::cerr << "Error: incorrect number of parameters!\n";
		std::cerr << "Usage: ./ircserv <port> <server password>" << std::endl;
		return (EXIT_FAILURE);
	}

	//start sighandler
	sigset_t mask;
	struct sigaction sa = {{0}, mask, 0, 0};
	sa.sa_handler = Server::sigHandler;

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
	//end sighandler

	try
	{
		Server server { av[1], av[2] };
		while (!g_quit)
			server.doPoll();
	}
	catch ( const std::exception& e )
	{
		std::cerr << e.what() << std::endl;
		return (EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}
