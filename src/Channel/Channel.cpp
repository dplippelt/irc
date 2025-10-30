/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Channel.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/28 10:31:56 by spyun         #+#    #+#                 */
/*   Updated: 2025/10/30 08:59:24 by spyun         ########   odam.nl         */
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

void Channel::setTopic(const std::string& topic)
{
	_topic = topic;
}

void Channel::setKey(const std::string& key)
{
	_key = key;
}

void Channel::setUserLimit(int limit)
{
	_userLimit = limit;
}

void Channel::setInviteOnly(bool value)
{
	_inviteOnly = value;
}

void Channel::setTopicRestricted(bool value)
{
	_topicRestricted = value;
}

void Channel::setHasKey(bool value)
{
	_hasKey = value;
	if (!value)
		_key.clear();
}

void Channel::setHasUserLimit(bool value)
{
	_hasUserLimit = value;
	if (!value)
		_userLimit = 0;
}

