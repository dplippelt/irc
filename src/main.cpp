/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 12:36:05 by dlippelt          #+#    #+#             */
/*   Updated: 2025/10/27 17:19:24 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "Server/Server.hpp"

int	main( int ac, char *av[] )
{
	if (ac != 3)
	{
		std::cerr << "Error: incorrect number of parameters!\n";
		std::cerr << "Usage: ./ircserv <port> <server password>" << std::endl;
		return (EXIT_FAILURE);
	}

	try
	{
		Server server { av[1], av[2] };
		while (true)
			server.doPoll();
	}
	catch ( const std::exception& e )
	{
		std::cerr << e.what() << std::endl;
		return (EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}
