/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: dlippelt <dlippelt@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/27 13:10:45 by dlippelt      #+#    #+#                 */
/*   Updated: 2026/01/26 15:58:30 by spyun         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "ResponseHandler.hpp"

/* ==================== Constructors & Destructors ==================== */

Server::~Server()
{
	for ( std::map<int, User*>::iterator it = m_users.begin(); it != m_users.end(); ++it )
		delete it->second;
	for ( std::map<std::string, Channel*>::iterator it = m_channels.begin(); it != m_channels.end(); ++it )
		delete it->second;
	if ( m_addr != nullptr )
		freeaddrinfo(m_addr);
	for ( auto pollfd : m_pollfds )
	{
		if ( close(pollfd.fd) == -1 )
			std::cerr << "Warning: failed to close socket file descriptor '" << pollfd.fd << "'" << std::endl;
	}
}

Server::Server( const std::string& port, std::string_view pw )
	: m_pw { pw }
{
	struct addrinfo	hints { AI_PASSIVE, AF_INET, SOCK_STREAM, 0, 0, NULL, NULL, NULL };

	validatePort(port);

	try
	{
		if ( getaddrinfo(NULL, port.data(), &hints, &m_addr) )
			throw std::runtime_error("Error: failed to get required address info");
		if ( m_addr->ai_next != NULL )
			std::cerr << "Warning: found more than one matching set of address info";

		m_listening_socket_fd = socket(m_addr->ai_family, m_addr->ai_socktype, m_addr->ai_protocol);
		if ( m_listening_socket_fd == -1 )
			throw std::runtime_error("Error: failed to create socket");
		int	opt {1};
		if ( setsockopt(m_listening_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
			throw std::runtime_error("Error: failed to set socket option SO_REUSEADDR");
		if ( fcntl(m_listening_socket_fd, F_SETFL, O_NONBLOCK) == -1 )
			throw std::runtime_error("Error: failed to set listening socket to non-blocking");
		m_pollfds.push_back( {m_listening_socket_fd, POLLIN, 0} );

		if ( bind(m_listening_socket_fd, m_addr->ai_addr, m_addr->ai_addrlen) == -1 )
			throw std::runtime_error("Error: failed to bind address to socket");
		if ( listen(m_listening_socket_fd, s_listen_backlog) == -1 )
			throw std::runtime_error("Error: failed to set socket as a passive socket listening for incoming connections");

		freeaddrinfo(m_addr);
		m_addr = nullptr;
	}
	catch (...)
	{
		if ( m_addr != nullptr )
			freeaddrinfo(m_addr);
		throw;
	}
}

/* ==================== Signal Handler ==================== */

void	Server::sigHandler( int signum )
{
	(void)signum;
	g_quit = 1;
}

void	Server::setupSigHandler()
{
	struct sigaction sa {};
	sigset_t mask {};

	sa.sa_handler = Server::sigHandler;
	sa.sa_mask = mask;

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
}

/* ==================== Initial Server Setup ==================== */

void	Server::validatePort( const std::string& port )
{
	int	port_int {};

	for ( char c : port )
	{
		if ( !std::isdigit(static_cast<unsigned char>(c)) )
			throw std::runtime_error("Error: port '" + port + "' contains non-digit characters");
	}

	try
	{
		port_int = std::stoi(port);
	}
	catch ( const std::exception& e )
	{
		throw std::runtime_error("Error: could not convert port '" + port + "' to integer");
	}

	if ( port_int < 1 || port_int > 65535 )
		throw std::runtime_error("Error: port '" + port + "' must be between 1 and 65535");
}

void	Server::acceptConn()
{
	#ifdef DEBUG
	std::cout << "Activity detected on listening socket!" << std::endl;
	#endif

	struct sockaddr_in	client_addr {};
	socklen_t			client_addr_len { sizeof(client_addr) };

	int client_fd { accept(m_listening_socket_fd, reinterpret_cast<struct sockaddr *>(&client_addr), &client_addr_len) };
	if ( client_fd == -1 )
		throw std::runtime_error("Error: failed to accept incoming connection");
	if ( fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1 )
		throw std::runtime_error("Error: failed to set client socket to non-blocking");

	User* newUser = new User(client_fd);
	m_users.insert( { client_fd, newUser } );
	m_pollfds.push_back( {client_fd, POLLIN, 0} );
	m_messagesList.emplace(client_fd, Parser{}); // [Takato]: added

	#ifdef DEBUG
	std::cout << "Accepted client connection (client fd: " << client_fd << ")" << std::endl;
	#endif
}

void	Server::doPoll()
{
	#ifdef DEBUG
	std::cout << "Polling..." << std::endl;
	#endif

	int ret = poll(m_pollfds.data(), m_pollfds.size(), -1);

	if ( ret == -1 )
	{
		switch (errno)
		{
		case EINTR:
			return;
		case EINVAL:
			throw std::runtime_error("Error: poll called with invalid argument");
		case ENOMEM:
			throw std::runtime_error("Error: failed to allocate memory");
		case EFAULT:
			throw std::runtime_error("Error: file descriptor(s) out of range");
		default:
			throw std::runtime_error("Error: an unknown error occurred during polling");
		}
	}

	for ( int i {static_cast<int>(m_pollfds.size()) - 1}; i >= 0; --i )
	{
		if ( m_pollfds[i].revents & (POLLHUP | POLLERR) )
		{
			if ( m_pollfds[i].fd == m_listening_socket_fd )
				throw std::runtime_error("Error: error on listening socket");
			else
			{
				std::map<int, User*>::iterator userIt = m_users.find(m_pollfds[i].fd);
				std::string quitMsg;
				if (userIt != m_users.end())
					quitMsg = userIt->second->getPrefix() + " QUIT :Connection reset by peer";
				removeClient(m_pollfds[i].fd, quitMsg);
				continue;
			}
		}

		if ( m_pollfds[i].revents & POLLIN )
		{
			if ( m_pollfds[i].fd == m_listening_socket_fd )
				acceptConn();
			else
				processClientAct(m_pollfds[i].fd);
		}

		if (m_pollfds[i].revents & POLLOUT)
		{
			trySendPendingData(m_pollfds[i].fd);
		}

	}
}

void	Server::removeClient( int client_fd, const std::string& quitMessage )
{
	#ifdef DEBUG
	std::cout << "Client disconnected (client fd: " << client_fd << ")" << std::endl;
	#endif

	auto it = m_users.find(client_fd);
	if ( it == m_users.end() )
		return;

	User* user = it->second;

	const std::vector<std::string>& channels = user->getChannels();
	for (size_t i = 0; i < channels.size(); ++i)
	{
		auto chanIt = m_channels.find(channels[i]);
		if (chanIt != m_channels.end())
		{
			Channel* channel = chanIt->second;
			if (!quitMessage.empty())
			{
				const std::map<int, User*>& members = channel->getMembers();
				for (auto memIt = members.begin(); memIt != members.end(); ++memIt)
				{
					if (memIt->first != client_fd)
						sendToClient(memIt->first, quitMessage);
				}
			}

			channel->removeMember(client_fd);

			if (channel->isEmpty())
			{
				m_channels.erase(chanIt);
				delete channel;

				#ifdef DEBUG
				std::cout << "Channel " << channels[i] << " deleted as it became empty." << std::endl;
				#endif
			}
		}
	}

	delete user;
	m_users.erase(it);
	m_messagesList.erase(client_fd);

	if ( close(client_fd) == -1 )
		std::cerr << "Warning: failed to close client file descriptor" << std::endl;

	for ( auto it {m_pollfds.begin()}; it != m_pollfds.end(); ++it )
	{
		if ( (*it).fd == client_fd )
		{
			m_pollfds.erase(it);
			break;
		}
	}
}

void	Server::processClientAct( int client_fd )
{
	#ifdef DEBUG
	std::cout << "Activity detected on client socket!" << std::endl;
	#endif

	char buffer[512];
	ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

	if ( bytes == 0 )
	{
		removeClient(client_fd);
		return;
	}
	if ( bytes == -1 )
	{
		if ( errno == EAGAIN || errno == EWOULDBLOCK )
			return;
		std::cerr << "Error while receiving message from client. Removing client..." << std::endl;
		removeClient(client_fd);
		return;
	}


	buffer[bytes] = '\0';
	processBuffer(buffer, client_fd);
}

Server::Server( const Server& ) = default;

Server& Server::operator=( const Server& ) = default;

/* ==================== Message Processing ==================== */

void	Server::processBuffer( const std::string& buffer, int client_fd )
{
	m_messagesList[client_fd].parse(buffer);

	std::list<Message> &messages { m_messagesList[client_fd].getMessages() };

	User* user {m_users[client_fd]};

	for ( auto& msg : messages )
	{
		Command command { *this, user, msg };
		command.executeCommand();
		if (m_users.find(client_fd) == m_users.end())
			return;
	}

	messages.clear();
}

/* ==================== Getters ==================== */

const std::map<int, User*>&	Server::getUsers() const
{
	return (m_users);
}

const std::map<std::string, Channel*>&	Server::getChannels() const
{
	return (m_channels);
}

std::map<int, User*>& Server::getUsers()
{
	return m_users;
}

std::map<std::string, Channel*>& Server::getChannels()
{
	return m_channels;
}

/* ==================== POLLOUT Management ==================== */

void	Server::enablePollOut(int fd)
{
	for ( size_t i {0}; i < m_pollfds.size(); ++i )
	{
		if ( m_pollfds[i].fd == fd )
		{
			m_pollfds[i].events |= POLLOUT;
			break;
		}
	}
}

void	Server::disablePollOut(int fd)
{
	for ( size_t i {0}; i < m_pollfds.size(); ++i )
	{
		if ( m_pollfds[i].fd == fd )
		{
			m_pollfds[i].events &= ~POLLOUT;
			break;
		}
	}
}

void	Server::trySendPendingData(int client_fd)
{
	std::map<int, User*>::iterator it = m_users.find(client_fd);
	if ( it == m_users.end() )
		return;

	User* user = it->second;
	if ( !user->hasPendingData() )
	{
		disablePollOut(client_fd);
		return;
	}

	std::string& buffer = user->getSendBuffer();

	ssize_t sent = send(client_fd, buffer.c_str(), buffer.length(), 0);

	if ( sent < 0)
	{
		if ( errno == EAGAIN || errno == EWOULDBLOCK )
			return;
		std::cerr << "Error sending to fd" << client_fd << ": " << strerror(errno) << std::endl;
		return;
 	}

	buffer.erase(0, sent);

	if ( buffer.empty() )
		disablePollOut(client_fd);
}

void	Server::sendToClient(int fd, const std::string& message)
{
	std::map<int, User*>::iterator it = m_users.find(fd);
	if ( it == m_users.end() )
		return;

	User* user = it->second;
	if ( user->hasPendingData() )
	{
		user->queueMessage(message);
		return;
	}
	ssize_t sent = send(fd, message.c_str(), message.length(), 0);
	if ( sent < 0)
	{
		if ( errno == EAGAIN || errno == EWOULDBLOCK )
		{
			user->queueMessage(message);
			enablePollOut(fd);
			return;
		}
		std::cerr << "Error sending to fd" << fd << ": " << strerror(errno) << std::endl;
		removeClient(fd);
		return;
	}
	if ( sent < static_cast<ssize_t>(message.length()) )
	{
		std::string remainder = message.substr(sent);
		user->queueMessage(remainder);
		enablePollOut(fd);
	}
}

#ifdef DEBUG
/* ==================== DEBUG Function ==================== */

void	Server::printModeStates() const
{
	for (auto it { m_channels.begin() }; it != m_channels.end(); ++it )
	{
		std::cout << "\nChannel name: " << it->first << std::endl;
		std::cout << "- Invite Only? " << (it->second->isInviteOnly() ? 1 : 0) << std::endl;
		std::cout << "- Topic Restriction Set? " << (it->second->isTopicRestricted() ? 1 : 0) << std::endl;
		std::cout << "- Key Set? " << (it->second->hasKey() ? 1 : 0) << std::endl;
		std::cout << "- User Limit Set? " << (it->second->hasUserLimit() ? 1 : 0) << std::endl;

		if (it->second->hasKey() || it->second->hasUserLimit())
		{
			std::cout << "Setting details:" << std::endl;
			if (it->second->hasKey())
				std::cout << "Channel key is '" << it->second->getKey() << "'" << std::endl;
			if (it->second->hasUserLimit())
				std::cout << "User limit is '" << it->second->getUserLimit() << "'" << std::endl;
		}

		std::cout << std::endl;

	}
}
#endif
