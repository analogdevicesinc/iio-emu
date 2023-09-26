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

#ifndef IIO_EMU_LOGGER_HPP
#define IIO_EMU_LOGGER_HPP

#include <string>
#include <vector>

namespace iio_emu {

#define _IIO_EMU_NO_LOG 0
#define _IIO_EMU_INFO 1
#define _IIO_EMU_DEBUG 2
#define _IIO_EMU_WARNING 3
#define _IIO_EMU_ERROR 4
#define _IIO_EMU_FATAL 5

enum LOG_LEVEL
{
	IIO_EMU_NO_LOG = _IIO_EMU_NO_LOG,
	IIO_EMU_INFO = _IIO_EMU_INFO,
	IIO_EMU_DEBUG = _IIO_EMU_DEBUG,
	IIO_EMU_WARNING = _IIO_EMU_WARNING,
	IIO_EMU_ERROR = _IIO_EMU_ERROR,
	IIO_EMU_FATAL = _IIO_EMU_FATAL
};

class Logger
{
public:
	Logger() = default;
	~Logger() = default;

	static void log(enum LOG_LEVEL level, const std::vector<std::string>& args);
	static bool verboseMode;
private:
	static inline std::string buildMsg(const std::vector<std::string>& args);
};
} // namespace iio_emu
#endif // IIO_EMU_LOGGER_HPP
