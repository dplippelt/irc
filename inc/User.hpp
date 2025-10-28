/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   User.hpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/27 16:14:15 by spyun         #+#    #+#                 */
/*   Updated: 2025/10/28 10:27:12 by spyun         ########   odam.nl         */
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
		// Network related
		int _fd;                        // Client socket file descriptor

		// User identification
		std::string _nickname;          // IRC nickname (required)
		std::string _username;          // Username (required)
		std::string _realname;          // Real name (optional)
		std::string _hostname;          // Hostname

		// Authentication state
		bool _authenticated;            // Authenticated via PASS command
		bool _registered;               // Registration complete (NICK + USER)

		// Channel related
		std::vector<std::string> _channels;  // List of joined channels

		// Buffer
		std::string _recvBuffer;        // Receive buffer

	public:
		// Constructor & Destructor
		User(int fd);
		~User();

		// Getter methods
		int getFd() const;
		const std::string& getNickname() const;
		const std::string& getUsername() const;
		const std::string& getRealname() const;
		const std::string& getHostname() const;
		bool isAuthenticated() const;
		bool isRegistered() const;
		const std::vector<std::string>& getChannels() const;

		// Setter methods
		void setNickname(const std::string& nickname);
		void setUsername(const std::string& username);
		void setRealname(const std::string& realname);
		void setHostname(const std::string& hostname);
		void setAuthenticated(bool auth);
		void setRegistered(bool reg);

		// Channel related methods
		void joinChannel(const std::string& channelName);
		void leaveChannel(const std::string& channelName);
		bool isInChannel(const std::string& channelName) const;

		// Buffer related
		std::string& getRecvBuffer();
		void clearRecvBuffer();

		// Utility
		std::string getPrefix() const;  // Format: :nick!user@host
};

#endif
