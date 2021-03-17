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

#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)

#include "network_win.hpp"

#include <algorithm>
#include <cerrno>
#include <winsock.h>
#include <winsock2.h>

using namespace iio_emu;

NetworkWin::NetworkWin() {}

NetworkWin::~NetworkWin() {}

int NetworkWin::open()
{
	int ret;
	WSADATA wsaData;
	ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != NO_ERROR) {
		return -1;
	}

	m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_listenSocket == INVALID_SOCKET) {
		WSACleanup();
		return -1;
	}

	bool yes = TRUE;
	if (setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&yes), sizeof(yes)) < 0) {
		close();
		return -1;
	}

	u_long nonBlock = 1;
	if (ioctlsocket(m_listenSocket, static_cast<long>(FIONBIO), &nonBlock) == SOCKET_ERROR) {
		close();
		return -1;
	}
	return 0;
}

int NetworkWin::close()
{
	if (m_listenSocket == INVALID_SOCKET) {
		return -1;
	}
	closesocket(m_listenSocket);
	WSACleanup();
	return 0;
}

int NetworkWin::bind(uint16_t port)
{
	if (m_listenSocket == INVALID_SOCKET) {
		return -1;
	}

	int addrlen = sizeof(m_address);

	m_address.sin_family = AF_INET;
	m_address.sin_addr.s_addr = htonl(INADDR_ANY);
	m_address.sin_port = htons(port);

	if (::bind(m_listenSocket, reinterpret_cast<PSOCKADDR>(&m_address), addrlen) == SOCKET_ERROR) {
		close();
		return -1;
	}
	return 0;
}

int NetworkWin::listen(int backlog)
{
	if (m_listenSocket == INVALID_SOCKET) {
		return -1;
	}
	if (::listen(m_listenSocket, backlog) == SOCKET_ERROR) {
		close();
		return -1;
	}
	return 0;
}

int NetworkWin::accept(SOCKET* clientSocket)
{
	if (m_listenSocket == INVALID_SOCKET) {
		return -1;
	}

	int ret;

	*clientSocket = ::accept(m_listenSocket, nullptr, nullptr);
	if (*clientSocket == INVALID_SOCKET) {
		close();
		return -1;
	}

	bool yes = TRUE;
	ret = setsockopt(*clientSocket, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<char*>(&yes), sizeof(yes));
	if (ret < 0) {
		closesocket(*clientSocket);
		return -1;
	}

	ret = setsockopt(*clientSocket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&yes), sizeof(yes));
	if (ret < 0) {
		closesocket(*clientSocket);
		return -1;
	}

	return 0;
}

int NetworkWin::checkForNewConnections()
{
	int ret, total;
	SOCKET new_socket;

	FD_ZERO(&m_fd_set);

	FD_SET(m_listenSocket, &m_fd_set);

	for (auto& client : clients) {
		FD_SET(client, &m_fd_set);
	}

	total = select(0, &m_fd_set, nullptr, nullptr, nullptr);

	if ((total == SOCKET_ERROR)) {
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

std::vector<int> NetworkWin::getActiveConnections()
{
	std::vector<int> activeConnections;
	for (auto client : clients) {
		if (FD_ISSET(client, &m_fd_set)) {
			activeConnections.push_back(static_cast<int>(client));
		}
	}
	return activeConnections;
}

void NetworkWin::disconnectSocket(int socket)
{
	clients.erase(std::remove(clients.begin(), clients.end(), socket), clients.end());
}

#endif
