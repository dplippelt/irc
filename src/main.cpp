/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 12:36:05 by dlippelt          #+#    #+#             */
/*   Updated: 2025/10/27 12:41:47 by dlippelt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

int	main( int ac, char *av[] )
{
	if (ac != 3)
	{
		std::cout << "Error: incorrect number of parameters!\n";
		std::cout << "Usage: ./ircserv <port> <server password>" << std::endl;
		return (EXIT_FAILURE);
	}

	(void)av;

	return (EXIT_SUCCESS);
}
