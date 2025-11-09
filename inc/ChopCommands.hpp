/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChopCommands.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmitsuya <tmitsuya@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/06 13:11:39 by tmitsuya          #+#    #+#             */
/*   Updated: 2025/11/07 17:01:48 by tmitsuya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHOPCOMMANDS_HPP
# define CHOPCOMMANDS_HPP

// To move command action related class
// # define ERR_UNKNOWNCOMMAND 421
/* 		421     ERR_UNKNOWNCOMMAND
                        "<command> :Unknown command"

                - Returned to a registered client to indicate that the
                  command sent is unknown by the server. */

// # define ERR_NONICKNAMEGIVEN 431
// # define ERR_ERRONEUSNICKNAME 432
// # define ERR_NICKNAMEINUSE 433
// # define ERR_NICKCOLLISION 436
// # define ERR_NEEDMOREPARAMS 461
/*        461    ERR_NEEDMOREPARAMS
              "<command> :Not enough parameters"

         - Returned by the server by numerous commands to
           indicate to the client that it didn't supply enough
           parameters.
 */
// # define ERR_ALREADYREGISTRED 462

class Server;
class Message;

class ChopCommands
{

public:

	ChopCommands() = delete;
	~ChopCommands() = delete;

	static void	mode(const Message &message, Server &server, User *user);

};




#endif
