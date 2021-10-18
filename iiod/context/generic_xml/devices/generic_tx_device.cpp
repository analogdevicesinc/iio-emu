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

#include "generic_tx_device.hpp"

#include "utils/logger.hpp"
#include "utils/utility.hpp"

using namespace iio_emu;

GenericTXDevice::GenericTXDevice(const char* device_id, const char* filePath)
	: m_filePath(filePath)
{
	auto tmpArray = new char[strlen(device_id) + 1];
	strncpy(tmpArray, device_id, strlen(device_id));
	m_device_id = tmpArray;
}

GenericTXDevice::~GenericTXDevice() { delete[] m_device_id; }

ssize_t GenericTXDevice::write_dev(const char* buf, size_t offset, size_t bytes_count)
{
	UNUSED(offset);

	m_output = std::fstream(m_filePath, std::ios::app | std::ios::binary);
	if (!m_output) {
		Logger::log(IIO_EMU_FATAL, {"Invalid file path: ", m_filePath});
		return -1;
	}

	m_output.write(buf, static_cast<long>(bytes_count));
	m_output.close();

	return static_cast<ssize_t>(bytes_count);
}

ssize_t GenericTXDevice::transfer_mem_to_dev(size_t bytes_count)
{
	UNUSED(bytes_count);
	return 0;
}

void GenericTXDevice::transfer_samples_to_RX_device(char* buf, size_t samples_count)
{
	UNUSED(buf);
	UNUSED(samples_count);
}

int32_t GenericTXDevice::open_dev(size_t sample_size, uint32_t mask, bool cyclic)
{
	UNUSED(sample_size);
	UNUSED(cyclic);
	m_mask = mask;
	return 0;
}

int32_t GenericTXDevice::close_dev() { return 0; }

int32_t GenericTXDevice::set_buffers_count(uint32_t buffers_count)
{
	UNUSED(buffers_count);
	return 0;
}

int32_t GenericTXDevice::get_mask(uint32_t* mask)
{
	*mask = m_mask;
	return 0;
}

int32_t GenericTXDevice::cancel_buffer() { return 0; }
