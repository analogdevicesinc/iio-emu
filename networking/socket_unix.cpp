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

#include "socket_unix.hpp"

#include <cerrno>
#include <sys/socket.h>
#include <unistd.h>

using namespace iio_emu;

SocketUnix::SocketUnix(int fd)
{
	m_fd = fd;
	m_disconnected = false;
}

size_t SocketUnix::getData(size_t len, char* buf)
{
	ssize_t ret;
	while (true) {
		ret = recv(m_fd, buf, len, 0);
		if (ret < 0 && errno == EAGAIN) {
			continue;
		}
		if (ret <= 0) {
			buf[0] = '\0';
			len = 0;
			close(m_fd);
			m_disconnected = true;
		}
		break;
	}

	return len;
}

void SocketUnix::write(const char* buf, size_t len)
{
	ssize_t ret;
	while (true) {
		ret = send(m_fd, buf, len, 0);
		if (ret < 0 && errno == EAGAIN) {
			continue;
		}
		if (ret < 0) {
			close(m_fd);
			m_disconnected = true;
		}
		break;
	}
}

bool SocketUnix::disconnected() const { return m_disconnected; }

int SocketUnix::getDescriptor() const { return m_fd; }

#endif
