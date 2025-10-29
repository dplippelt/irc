/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Channel.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/28 10:31:56 by spyun         #+#    #+#                 */
/*   Updated: 2025/10/29 12:09:31 by seungah       ########   odam.nl         */
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

const std::string& Channel::getTopic() const
{
	return _topic;
}

const std::string& Channel::getKey() const
{
	return _key;
}

size_t Channel::getMemberCount() const
{
	return _members.size();
}

int Channel::getUserLimit() const
{
	return _userLimit;
}

time_t Channel::getCreationTime() const
{
	return _creationTime;
}

bool Channel::isInviteOnly() const
{
	return _inviteOnly;
}

bool Channel::isTopicRestricted() const
{
	return _topicRestricted;
}

bool Channel::hasKey() const
{
	return _hasKey;
}

bool Channel::hasUserLimit() const
{
	return _hasUserLimit;
}

