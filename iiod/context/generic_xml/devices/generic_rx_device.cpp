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

#include "generic_rx_device.hpp"

#include "utils/logger.hpp"
#include "utils/utility.hpp"

using namespace iio_emu;

const static int BUF_SIZE = 4096;

GenericRXDevice::GenericRXDevice(const char* device_id, const char* filePath)
	: m_filePath(filePath)
{
	auto tmpArray = new char[strlen(device_id) + 1];
	strncpy(tmpArray, device_id, strlen(device_id));
	m_device_id = tmpArray;
}

GenericRXDevice::~GenericRXDevice() { delete[] m_device_id; }
#include "iostream"
ssize_t GenericRXDevice::read_dev(char* pbuf, size_t offset, size_t bytes_count)
{
	UNUSED(offset);

	m_input = std::fstream(m_filePath, std::ios::in | std::ios::binary);
	if (!m_input) {
		Logger::log(IIO_EMU_FATAL, {"Invalid file path: ", m_filePath});
		return -1;
	}

	m_input.seekg(std::ios::beg);

	m_input.read(pbuf, static_cast<long>(bytes_count));

	for (auto i = static_cast<size_t>(m_input.gcount()); i < bytes_count; i++) {
		pbuf[i] = 0;
	}

	std::fstream tmpFile("iio_emu_tmp_file.txt", std::ios::out | std::ios::binary);
	copyFileData(m_input, tmpFile);
	tmpFile.close();

	m_input.close();
	m_input.open(m_filePath, std::ofstream::out | std::ofstream::trunc);
	m_input.close();

	tmpFile.open("iio_emu_tmp_file.txt", std::ios::in | std::ios::binary);
	m_input.open(m_filePath, std::ios::out | std::ios::binary);
	tmpFile.seekg(std::ios::beg);

	copyFileData(tmpFile, m_input);

	m_input.close();
	tmpFile.close();
	std::remove("iio_emu_tmp_file.txt");

	return static_cast<ssize_t>(bytes_count);
}

void GenericRXDevice::connectDevice(unsigned short channel_in, AbstractDeviceOut* deviceOut, unsigned short channel_out)
{
	UNUSED(channel_in);
	UNUSED(deviceOut);
	UNUSED(channel_out);
}

ssize_t GenericRXDevice::transfer_dev_to_mem(size_t bytes_count)
{
	UNUSED(bytes_count);
	return 0;
}

void GenericRXDevice::copyFileData(std::fstream& in, std::fstream& out)
{
	char buf[BUF_SIZE];

	do {
		in.read(&buf[0], BUF_SIZE);
		out.write(&buf[0], in.gcount());
	} while (in.gcount() > 0);
}

int32_t GenericRXDevice::open_dev(size_t sample_size, uint32_t mask, bool cyclic)
{
	UNUSED(sample_size);
	UNUSED(cyclic);
	m_mask = mask;
	return 0;
}

int32_t GenericRXDevice::close_dev() { return 0; }

int32_t GenericRXDevice::set_buffers_count(uint32_t buffers_count)
{
	UNUSED(buffers_count);
	return 0;
}

int32_t GenericRXDevice::get_mask(uint32_t* mask)
{
	*mask = m_mask;
	return 0;
}

int32_t GenericRXDevice::cancel_buffer() { return 0; }
