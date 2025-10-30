/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Channel.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/28 10:31:56 by spyun         #+#    #+#                 */
/*   Updated: 2025/10/30 11:17:37 by spyun         ########   odam.nl         */
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

void Channel::addMember(User* user)
{
	if (user)
	{
		_members[user->getFd()] = user;
		if (_members.size() == 1)
		{
			_operators.insert(user->getFd());
		}
	}
}

void Channel::removeMember(int fd)
{
	_members.erase(fd);
	_operators.erase(fd);
	_inviteList.erase(fd);
}

bool Channel::isMember(int fd) const
{
	return _members.find(fd) != _members.end();
}

User* Channel::getMember(int fd) const
{
	std::map<int, User*>::const_iterator it = _members.find(fd);
	if (it != _members.end())
		return it->second;
	return nullptr;
}

const std::map<int, User*>& Channel::getMembers() const
{
	return _members;
}

void Channel::addOperator(int fd)
{
	if(isMember(fd))
		_operators.insert(fd);
}

void Channel::removeOperator(int fd)
{
	_operators.erase(fd);
}

bool Channel::isOperator(int fd) const
{
	return _operators.find(fd) != _operators.end();
}

const std::set<int>& Channel::getOperators() const
{
	return _operators;
}

void Channel::addInvite(int fd)
{
	_inviteList.insert(fd);
}

void Channel::removeInvite(int fd)
{
	_inviteList.erase(fd);
}

bool Channel::isInvited(int fd) const
{
	return _inviteList.find(fd) != _inviteList.end();
}

void Channel::broadcast(const std::string& message, int excludeFd)
{
	for (std::map<int, User*>::const_iterator it = _members.begin(); it != _members.end(); ++it)
	{
		if (it->first != excludeFd)
		{
			(void)message;
			//placeholder for sending message to user
		}
	}
}

std::string Channel::getMemberList() const
{
	std::string memberList;
	for (std::map<int, User*>::const_iterator it = _members.begin(); it != _members.end(); ++it)
	{
		if (!memberList.empty())
			memberList += " ";
		if (isOperator(it->first))
			memberList += "@";
		memberList += it->second->getNickname();
	}
	return memberList;
}


bool Channel::isEmpty() const
{
	return _members.empty();
}
