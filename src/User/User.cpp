/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   User.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: spyun <spyun@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/27 16:15:28 by spyun         #+#    #+#                 */
/*   Updated: 2025/10/27 17:35:10 by seungah       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "User.hpp"

User::User(int fd) 
	: _fd(fd)
	, _authenticated(false)
	, _registered(false) {}

User::~User() {}