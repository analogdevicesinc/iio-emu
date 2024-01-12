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

#include "factory_ops.hpp"

#include "abstract_ops.hpp"
#include "iiod/context/adalm2000/adalm2000_context.hpp"
#include "iiod/context/generic_xml/generic_xml_context.hpp"

#include <utils/logger.hpp>
#include <iostream>

constexpr uint16_t DEFAULT_PORT = 30431;


using namespace iio_emu;

bool is_number(const char* s)
{
	while (*s) {
		if (!isdigit(*s))
			return false;
		s++;
	}

	return true;
}

AbstractOps* FactoryOps::buildOps(const char* type, std::vector<const char*>& args)
{
	AbstractOps* iiodOpsAbstract;

	if (!strncmp(type, "generic", sizeof("generic") - 1)) {
		if (args.empty()) {
			Logger::log(IIO_EMU_FATAL, {"XML path required"});
			return nullptr;
		}
		iiodOpsAbstract = new GenericXmlContext(args);

		const char* last_item = args.back();
		if (is_number(last_item)) {
			unsigned long i_port = std::stoul(last_item);
			if (i_port > 65535) {
				Logger::log(IIO_EMU_FATAL, {"Invalid port number"});
				return nullptr;
			}
			iiodOpsAbstract->port = static_cast<uint16_t>(i_port);
			Logger::log(IIO_EMU_INFO, {"Using custom port ", std::to_string(iiodOpsAbstract->port)});
		}
		else
			iiodOpsAbstract->port = DEFAULT_PORT;

	} else if (!strncmp(type, "adalm2000", sizeof("adalm2000") - 1)) {
		iiodOpsAbstract = new Adalm2000Context();
	} else {
		return nullptr;
	}

	return iiodOpsAbstract;
}
