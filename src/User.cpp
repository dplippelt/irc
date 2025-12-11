/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   User.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: dlippelt <dlippelt@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/27 16:15:28 by spyun         #+#    #+#                 */
/*   Updated: 2025/12/11 14:07:48 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "User.hpp"

User::User(int fd)
	: _fd(fd)
	, _authenticated(false)
	, _registered(false)
	, _passwordProvided(false)
	, _hasNickname(false)
	, _hasUsername(false) {}

User::~User() {}

int User::getFd() const
{
	return _fd;
}

const std::string& User::getNickname() const
{
	return _nickname;
}

const std::string& User::getUsername() const
{
	return _username;
}

const std::string& User::getRealname() const
{
	return _realname;
}

const std::string& User::getHostname() const
{
	return _hostname;
}

std::string User::getSendBuffer() const
{
	return _sendBuffer;
}

bool User::hasPendingData() const
{
	return !_sendBuffer.empty();
}

void User::queueMessage(const std::string& message)
{
	_sendBuffer += message;
}

void User::clearSendBuffer()
{
	_sendBuffer.clear();
}

bool User::hasProvidedPassword() const
{
	return _passwordProvided;
}

bool User::hasNickname() const
{
	return _hasNickname;
}

bool User::hasUsername() const
{
	return _hasUsername;
}

bool User::isAuthenticated() const
{
	return _authenticated;
}

bool User::isRegistered() const
{
	return _registered;
}

const std::vector<std::string>& User::getChannels() const
{
	return _channels;
}

void User::setNickname(const std::string& nickname)
{
	_nickname = nickname;
}

void User::setUsername(const std::string& username)
{
	_username = username;
}

void User::setRealname(const std::string& realname)
{
	_realname = realname;
}

void User::setHostname(const std::string& hostname)
{
	_hostname = hostname;
}

void User::setPasswordProvided(bool provided)
{
	_passwordProvided = provided;
}

void User::setHasNickname(bool has)
{
	_hasNickname = has;
}

void User::setHasUsername(bool has)
{
	_hasUsername = has;
}

void User::setAuthenticated(bool auth)
{
	_authenticated = auth;
}

void User::setRegistered(bool reg)
{
	_registered = reg;
}

void User::joinChannel(const std::string& channelName)
{
	if (!isInChannel(channelName))
		_channels.push_back(channelName);
}

void User::leaveChannel(const std::string& channelName)
{
	std::vector<std::string>::iterator it = std::find(_channels.begin(), _channels.end(), channelName);
	if (it != _channels.end())
		_channels.erase(it);
}

bool User::isInChannel(const std::string& channelName) const
{
	return std::find(_channels.begin(), _channels.end(), channelName) != _channels.end();
}

std::string& User::getRecvBuffer()
{
	return _recvBuffer;
}

void User::clearRecvBuffer()
{
	_recvBuffer.clear();
}

std::string User::getPrefix() const
{
	std::string prefix = ":" + _nickname;
	if(!_nickname.empty())
		prefix += "!~" + _username;
	if (!_hostname.empty())
		prefix += "@" + _hostname;
	return prefix;
}
