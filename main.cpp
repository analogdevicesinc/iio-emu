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

#include "networking/tcp_server.hpp"
#include "utils/logger.hpp"

#include <iostream>
#include <vector>
extern "C"
{
#include <getopt.h>
}
#include <inttypes.h> /* strtoimax */
//default port value
uint16_t port = 30431;

uint16_t strToUint16T(const char *str) {
    char *end;
    errno = 0;
    intmax_t val = strtoimax(str, &end, 10);
    if (errno == ERANGE || val < 0 || val > UINT16_MAX || end == str || *end != '\0'){
        iio_emu::Logger::log(iio_emu::IIO_EMU_INFO, {"TCP Port value invalid ", str});
		exit(1);
    } 
	iio_emu::Logger::log(iio_emu::IIO_EMU_INFO, {"TCP Port: ", str});
    return static_cast<uint16_t>(val);
}

void handleOptions(int argc, char* argv[])
{
	int retOption = 0;
	static struct option longOptions[] = {
		{"help", no_argument, 0, 'h'}, {"list", no_argument, 0, 'l'}, {"verbose", no_argument, 0, 'v'}, {"port",  required_argument, 0, 'p'}};
	retOption = getopt_long(argc, argv, "hlvp:", longOptions, NULL);

	switch (retOption) {
	case 'h':
		iio_emu::Logger::log(iio_emu::IIO_EMU_INFO, {"Options:"});
		iio_emu::Logger::log(iio_emu::IIO_EMU_INFO,
				     {"-h, ", "--help;", "     Displays help on commandline options"});
		iio_emu::Logger::log(iio_emu::IIO_EMU_INFO, {"-l, ", "--list;", "     Displays the calling options"});
		iio_emu::Logger::log(iio_emu::IIO_EMU_INFO, {"-p, ", "--port;", "     Set TCP server port"});
		iio_emu::Logger::log(iio_emu::IIO_EMU_INFO,
				     {"-v, ", "--verbose;", "  Running in verbose mode; Must to be put at the end"});
		exit(0);
	case 'l':
		iio_emu::Logger::log(iio_emu::IIO_EMU_INFO, {"iio-emu adalm2000"});
		iio_emu::Logger::log(
			iio_emu::IIO_EMU_INFO,
			{"iio-emu generic <path_to_XML> <device_id>@<file_path>;", " <path_to_XML> is mandatory"});
		exit(0);
	case 'v':
		iio_emu::Logger::verboseMode = true;
		break;
	case 'p':
		if (optarg) {
			port = strToUint16T(optarg);
		} else {
			iio_emu::Logger::log(iio_emu::IIO_EMU_INFO, {"Port value invalid "});
			exit(0);
		}
		break;
	}
}

int main(int argc, char* argv[])
{
	handleOptions(argc, argv);
	if (iio_emu::Logger::verboseMode) {
		std::string lastArg = std::string(argv[argc - 1]);
		bool isVerboseAtTheEnd = (lastArg.compare("--verbose") == 0 || lastArg.compare("-v") == 0);
		if (!isVerboseAtTheEnd) {
			iio_emu::Logger::log(iio_emu::IIO_EMU_FATAL, {"Verbose option must be put at the end!"});
			exit(1);
		}
	}
	if (argc < 2 || (argc == 2 && iio_emu::Logger::verboseMode)) {
		iio_emu::Logger::log(iio_emu::IIO_EMU_FATAL, {"No server type provided"});
		exit(1);
	}
	iio_emu::Logger::log(iio_emu::IIO_EMU_INFO, {"Virtual device: ", argv[1]});

	std::vector<const char*> args;
	for (int i = 2; i < argc; i++) {
		if (iio_emu::Logger::verboseMode && (i == (argc - 1))) {
			break;
		}
		args.push_back(argv[i]);
	}

	iio_emu::TcpServer server(argv[1], args);
	auto ret = server.start(port);
	exit(ret);
}
