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

#include "tcp_server.hpp"
#include <iiod/ops/abstract_ops.hpp>
#include "iiod/ops/factory_ops.hpp"
#if !defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
#include "network_unix.hpp"
#include "socket_unix.hpp"
#else
#include "network_win.hpp"
#include "socket_win.hpp"
#endif
#include "utils/utility.hpp"
#include <iostream>
#include <csignal>
#include <vector>
extern "C" {
#include <tinyiiod/tinyiiod.h>
}

constexpr int PORT = 30431;
constexpr int BACKLOG = 64;

bool running = true;

using namespace iio_emu;

TcpServer::TcpServer(const char *type, std::vector<const char *> &args)
{
	FactoryOps factory;
	m_ops = factory.buildOps(type, args);
	if (m_ops == nullptr) {
		std::cerr << "No such device" << std::endl;
		exit(1);
	}
	m_iiod = tinyiiod_create(m_ops->getIIODOps());
}

TcpServer::~TcpServer()
{
	if (m_iiod) {
		tinyiiod_destroy(m_iiod);
	}

	delete m_ops;
}

bool TcpServer::start()
{
	int ret;
	bool errorOccured = false;

	signal(SIGINT, TcpServer::stop);
	signal(SIGILL, TcpServer::stop);
	signal(SIGTERM, TcpServer::stop);
	signal(SIGSEGV, TcpServer::stop);

#if !defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
	signal(SIGSTOP, TcpServer::stop);
	signal(SIGPIPE, SIG_IGN);
#endif

	NetworkInterface *networkInterface;
#if !defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
	networkInterface = new NetworkUnix();
#else
	networkInterface = new NetworkWin();
#endif
	if (networkInterface == nullptr) {
		return false;
	}

	//create listen socket
	ret = networkInterface->open();
	if (ret < 0) {
		std::cerr << "Socket cannot be created: " << strerror(errno) << std::endl;
		delete networkInterface;
		return false;
	}

	ret = networkInterface->bind(PORT);
	if (ret < 0) {
		std::cerr << "Bind failed: " << strerror(errno) << std::endl;
		delete networkInterface;
		return false;
	}

	ret = networkInterface->listen(BACKLOG);
	if (ret < 0) {
		std::cerr << "Listen failed: " << strerror(errno) << std::endl;
		delete networkInterface;
		return false;
	}
	std::cout << "Waiting for connections ..." << std::endl;

	while (running) {
		ret = networkInterface->checkForNewConnections();
		if (ret < 0) {
			std::cerr << "New connection failed: " << strerror(errno) << std::endl;
			errorOccured = true;
			break;
		}

		// handle active connections
		auto activeConnections = networkInterface->getActiveConnections();
		for (auto client : activeConnections) {
#if !defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
			SocketUnix socket(client);
#else
			SocketWin socket(client);
#endif
			m_ops->setCurrentSocket(&socket);
			tinyiiod_read_command(m_iiod);
			if (socket.disconnected()) {
				m_ops->socketDisconnected(client);
				networkInterface->disconnectSocket(client);
			}
		}
	}
	if (!errorOccured) {
		networkInterface->close();
	}
	delete networkInterface;
	std::cout << "Server stopped" << std::endl;
	return !errorOccured;
}

void TcpServer::stop(int signum)
{
	UNUSED(signum);
	running = false;
}
