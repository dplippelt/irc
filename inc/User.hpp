/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   User.hpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/27 16:14:15 by spyun         #+#    #+#                 */
/*   Updated: 2025/12/11 16:31:24 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef USER_HPP
#define USER_HPP

#include <string>
#include <vector>
#include <algorithm>

class User
{
	private:
		int _fd;

		std::string _nickname;
		std::string _username;
		std::string _realname;
		std::string _hostname;

		bool _authenticated;
		bool _registered;
		bool _passwordProvided;
		bool _hasNickname;
		bool _hasUsername;

		std::vector<std::string> _channels;

		std::string _recvBuffer;
		std::string _sendBuffer;

	public:
		User(int fd);
		~User();

		int getFd() const;
		const std::string& getNickname() const;
		const std::string& getUsername() const;
		const std::string& getRealname() const;
		const std::string& getHostname() const;
		std::string& getSendBuffer();
		bool hasPendingData() const;
		void queueMessage(const std::string& message);
		void clearSendBuffer();
		bool hasProvidedPassword() const;
		bool hasNickname() const;
		bool hasUsername() const;
		bool isAuthenticated() const;
		bool isRegistered() const;
		const std::vector<std::string>& getChannels() const;

		void setNickname(const std::string& nickname);
		void setUsername(const std::string& username);
		void setRealname(const std::string& realname);
		void setHostname(const std::string& hostname);
		void setPasswordProvided(bool provided);
		void setHasNickname(bool has);
		void setHasUsername(bool has);
		void setAuthenticated(bool auth);
		void setRegistered(bool reg);

		void joinChannel(const std::string& channelName);
		void leaveChannel(const std::string& channelName);
		bool isInChannel(const std::string& channelName) const;

		std::string& getRecvBuffer();
		void clearRecvBuffer();

		std::string getPrefix() const;
};

#endif
