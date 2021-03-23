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

#include "logger.hpp"

#include "utils/utility.hpp"

#include <iostream>
#include <sstream>

void iio_emu::Logger::log(iio_emu::LOG_LEVEL level, const std::vector<std::string>& args)
{
	auto msg = buildMsg(args);
	switch (level) {
	case IIO_EMU_NO_LOG:
		UNUSED(msg);
		break;
	case IIO_EMU_INFO:
#if IIO_EMU_LOG_LEVEL >= _IIO_EMU_INFO
		std::cout << msg << std::endl;
#endif
		break;
	case IIO_EMU_DEBUG:
#if IIO_EMU_LOG_LEVEL >= _IIO_EMU_DEBUG
		std::cout << "DEBUG: " << msg << std::endl;
#endif
		break;
	case IIO_EMU_WARNING:
#if IIO_EMU_LOG_LEVEL >= _IIO_EMU_WARNING
		std::cerr << "WARNING: " << msg << std::endl;
#endif
		break;
	case IIO_EMU_ERROR:
#if IIO_EMU_LOG_LEVEL >= _IIO_EMU_ERROR
		std::cerr << "ERROR: " << msg << std::endl;
#endif
		break;
	case IIO_EMU_FATAL:
#if IIO_EMU_LOG_LEVEL >= _IIO_EMU_FATAL
		std::cerr << "FATAL: " << msg << std::endl;
#endif
		break;
	}
}

inline std::string iio_emu::Logger::buildMsg(const std::vector<std::string>& agrs)
{
	std::stringstream ss;
	for (auto& arg : agrs) {
		ss << arg;
	}
	return ss.str();
}
