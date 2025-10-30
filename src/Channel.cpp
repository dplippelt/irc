/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Channel.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/28 10:31:56 by spyun         #+#    #+#                 */
/*   Updated: 2025/10/28 13:58:29 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(const std::string& name)
	: _name(name)
	, _inviteOnly(false)
	, _topicRestricted(true)
	, _hasKey(false)
	, _hasUserLimit(false)
	, _userLimit(0)
	, _creationTime(std::time(NULL)) {}


Channel::~Channel() {}

const std::string& Channel::getName() const
{
	return _name;
}
