/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Channel.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/28 10:31:59 by spyun         #+#    #+#                 */
/*   Updated: 2025/11/07 14:05:41 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <map>
#include <set>
#include <vector>
#include <ctime>
#include "User.hpp"

class Channel
{
	private:
		std::string _name;                      // Channel name (starts with #)
		std::string _topic;                     // Channel topic
		std::string _key;                       // Channel password (mode +k)

		// Members management
		std::map<int, User*> _members;          // fd -> User pointer
		std::set<int> _operators;               // fd of operators
		std::set<int> _inviteList;              // fd of invited users (mode +i)

		// Channel modes
		bool _inviteOnly;                       // +i: Invite-only channel
		bool _topicRestricted;                  // +t: Only operators can change topic
		bool _hasKey;                           // +k: Channel has password
		bool _hasUserLimit;                     // +l: Channel has user limit
		int _userLimit;                         // Maximum number of users

		// Creation timestamp
		time_t _creationTime;

	public:
		// Constructor & Destructor
		Channel(const std::string& name);
		~Channel();

		// Getter methods
		const std::string& getName() const;
		const std::string& getTopic() const;
		const std::string& getKey() const;
		size_t getMemberCount() const;
		int getUserLimit() const;
		time_t getCreationTime() const;
		bool isInviteOnly() const;
		bool isTopicRestricted() const;
		bool hasKey() const;
		bool hasUserLimit() const;

		// Setter methods
		void setTopic(const std::string& topic);
		void setKey(const std::string& key);
		void setUserLimit(int limit);
		void setInviteOnly(bool value);
		void setTopicRestricted(bool value);
		void setHasKey(bool value);
		void setHasUserLimit(bool value);

		// Member management
		void addMember(User* user);
		void removeMember(int fd);
		bool isMember(int fd) const;
		User* getMember(int fd) const;
		const std::map<int, User*>& getMembers() const;

		// Operator management
		void addOperator(int fd);
		void removeOperator(int fd);
		bool isOperator(int fd) const;
		const std::set<int>& getOperators() const;

		// Invite management (for +i mode)
		void addInvite(int fd);
		void removeInvite(int fd);
		bool isInvited(int fd) const;

		// Utility methods
		void broadcast(const std::string& message, int excludeFd = -1);
		std::string getMemberList() const;      // Get list of nicknames
		bool isEmpty() const;
};

#endif
