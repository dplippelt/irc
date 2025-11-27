/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 10:31:59 by spyun             #+#    #+#             */
/*   Updated: 2025/11/27 11:33:49 by dlippelt         ###   ########.fr       */
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
		std::string _name;
		std::string _topic;
		std::string _key;

		std::map<int, User*> _members;
		std::set<int> _operators;
		std::set<int> _inviteList;

		bool _inviteOnly;
		bool _topicRestricted;
		bool _hasKey;
		bool _hasUserLimit;
		int _userLimit;

		time_t _creationTime;

	public:
		Channel(const std::string& name);
		~Channel();

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

		void setTopic(const std::string& topic);
		void setKey(const std::string& key);
		void setUserLimit(int limit);
		void setInviteOnly(bool value);
		void setTopicRestricted(bool value);
		void setHasKey(bool value);
		void setHasUserLimit(bool value);

		void addMember(User* user);
		void removeMember(int fd);
		bool isMember(int fd) const;
		User* getMember(int fd) const;
		const std::map<int, User*>& getMembers() const;

		void addOperator(int fd);
		void removeOperator(int fd);
		bool isOperator(int fd) const;
		const std::set<int>& getOperators() const;

		void addInvite(int fd);
		void removeInvite(int fd);
		bool isInvited(int fd) const;

		void broadcast(const std::string& message, int excludeFd = -1);
		std::string getMemberList() const;
		bool isEmpty() const;

		std::string getModeString() const;
};

#endif
