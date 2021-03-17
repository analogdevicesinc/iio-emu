/*
 * ADIBSD License
 *
 * Copyright (c) 2021 Analog Devices Inc.
 * All rights reserved.
 *
 * This file is part of iio-emu
 * (see http://www.github.com/analogdevicesinc/iio-emu).
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *     - Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     - Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     - Neither the name of Analog Devices, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *     - The use of this software may or may not infringe the patent rights
 *       of one or more patent holders.  This license does not release you
 *       from the requirement that you obtain separate licenses from these
 *       patent holders to use this software.
 *     - Use of the software either in source or binary form, must be run
 *       on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.
 *
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, INTELLECTUAL PROPERTY
 * RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#if !defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)

#include "network_unix.hpp"

#include <algorithm>
#include <cerrno>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace iio_emu;

NetworkUnix::NetworkUnix() { m_address = new struct sockaddr_in; }

NetworkUnix::~NetworkUnix() { delete m_address; }

int NetworkUnix::open()
{
	int yes = 1;
	m_listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_listenSocket <= 0) {
		return -1;
	}

	if (fcntl(m_listenSocket, F_SETFL, O_NONBLOCK) < 0) {
		close();
		return -1;
	}

	if (setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
		close();
		return -1;
	}
	return 0;
}

int NetworkUnix::close()
{
	if (m_listenSocket <= 0) {
		return -1;
	}

	::close(m_listenSocket);
	return 0;
}

int NetworkUnix::bind(uint16_t port)
{
	if (m_listenSocket <= 0) {
		return -1;
	}

	socklen_t addrlen = sizeof(*m_address);

	m_address->sin_family = AF_INET;
	m_address->sin_addr.s_addr = INADDR_ANY;
	m_address->sin_port = htons(port);
	if (::bind(m_listenSocket, reinterpret_cast<struct sockaddr*>(m_address), addrlen) < 0) {
		close();
		return -1;
	}
	return 0;
}

int NetworkUnix::listen(int backlog)
{
	if (m_listenSocket <= 0) {
		return -1;
	}
	if (::listen(m_listenSocket, backlog) < 0) {
		close();
		return -1;
	}
	return 0;
}

int NetworkUnix::accept(int* clientSocket)
{
	if (m_listenSocket <= 0) {
		return -1;
	}

	socklen_t addrlen = sizeof(*m_address);
	int ret, yes = 1, keepalive_intvl = 10, keepalive_probes = 6;

	*clientSocket = ::accept(m_listenSocket, reinterpret_cast<struct sockaddr*>(m_address), &addrlen);
	if (*clientSocket < 0) {
		return -EAGAIN;
	}

	ret = setsockopt(static_cast<int>(*clientSocket), SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes));
	if (ret < 0) {
		return -1;
	}
	ret = setsockopt(static_cast<int>(*clientSocket), IPPROTO_TCP, TCP_KEEPCNT, &keepalive_probes,
			 sizeof(keepalive_probes));
	if (ret < 0) {
		return -1;
	}

	ret = setsockopt(static_cast<int>(*clientSocket), IPPROTO_TCP, TCP_KEEPINTVL, &keepalive_intvl,
			 sizeof(keepalive_intvl));
	if (ret < 0) {
		return -1;
	}
	ret = setsockopt(static_cast<int>(*clientSocket), IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));
	if (ret < 0) {
		return -1;
	}

	return 0;
}

int NetworkUnix::checkForNewConnections()
{
	int ret, maxFd, total, new_socket;

	FD_ZERO(&m_fd_set);

	FD_SET(m_listenSocket, &m_fd_set);
	maxFd = m_listenSocket;

	for (auto& client : clients) {
		FD_SET(client, &m_fd_set);
	}

	if (!clients.empty()) {
		auto it = std::max_element(std::begin(clients), std::end(clients));
		maxFd = *it;
	}

	total = select(maxFd + 1, &m_fd_set, nullptr, nullptr, nullptr);

	if ((total < 0) && (errno != EINTR)) {
		close();
		return -1;
	}

	if (FD_ISSET(m_listenSocket, &m_fd_set)) {
		ret = this->accept(&new_socket);
		if (ret == EAGAIN) {
			return 0;
		}
		if (ret < 0) {
			close();
			return ret;
		}
		clients.push_back(new_socket);
	}
	return 0;
}

std::vector<int> NetworkUnix::getActiveConnections()
{
	std::vector<int> activeConnections;
	for (auto client : clients) {
		if (FD_ISSET(client, &m_fd_set)) {
			activeConnections.push_back(client);
		}
	}
	return activeConnections;
}

void NetworkUnix::disconnectSocket(int socket)
{
	clients.erase(std::remove(clients.begin(), clients.end(), socket), clients.end());
}

#endif
