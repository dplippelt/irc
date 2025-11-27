/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   IrcNumericCodes.hpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/24 13:55:19 by spyun         #+#    #+#                 */
/*   Updated: 2025/11/27 11:31:10 by seungah       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRCNUMERICCODES_HPP
#define IRCNUMERICCODES_HPP

enum IrcNumericCodes
{
	RPL_WELCOME = 001,
	RPL_YOURHOST = 002,
	RPL_CREATED = 003,
	RPL_MYINFO = 004,

	RPL_WHOISUSER = 311,
	RPL_WHOISSERVER = 312,
	RPL_ENDOFWHOIS = 318,
	RPL_WHOISCHANNELS = 319,
	RPL_CHANNELMODEIS = 324,
	RPL_NOTOPIC = 331,
	RPL_TOPIC = 332,
	RPL_INVITING = 341,
	RPL_NAMREPLY = 353,
	RPL_ENDOFNAMES = 366,

	ERR_NOSUCHNICK = 401,
	ERR_NOSUCHCHANNEL = 403,
	ERR_CANNOTSENDTOCHAN = 404,
	ERR_TOOMANYCHANNELS = 405,
	ERR_NORECIPIENT = 411,
	ERR_NOTEXTTOSEND = 412,
	ERR_NONICKNAMEGIVEN = 431,
	ERR_ERRONEUSNICKNAME = 432,
	ERR_NICKNAMEINUSE = 433,
	ERR_USERNOTINCHANNEL = 441,
	ERR_NOTONCHANNEL = 442,
	ERR_USERONCHANNEL = 443,
	ERR_NOTREGISTERED = 451,
	ERR_NEEDMOREPARAMS = 461,
	ERR_ALREADYREGISTRED = 462,
	ERR_PASSWDMISMATCH = 464,
	ERR_KEYSET = 467,
	ERR_CHANNELISFULL = 471,
	ERR_UNKNOWNMODE = 472,
	ERR_INVITEONLYCHAN = 473,
	ERR_BADCHANNELKEY = 475,
	ERR_CHANOPRIVSNEEDED = 482,
};

#endif
