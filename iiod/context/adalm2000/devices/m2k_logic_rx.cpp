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

#include "m2k_logic_rx.hpp"

#include "utils/attr_ops_xml.hpp"
#include "utils/utility.hpp"

using namespace iio_emu;

M2kLogicRX::M2kLogicRX(const char* device_id, struct _xmlDoc* doc)
{
	m_device_id = device_id;
	m_doc = doc;

	m_connections = std::vector<std::pair<AbstractDeviceOut*, unsigned short>>(1);
}

M2kLogicRX::~M2kLogicRX() {}

int32_t M2kLogicRX::open_dev(size_t sample_size, uint32_t mask, bool cyclic)
{
	UNUSED(sample_size);
	UNUSED(mask);
	UNUSED(cyclic);
	return 0;
}

int32_t M2kLogicRX::close_dev() { return 0; }

int32_t M2kLogicRX::set_buffers_count(uint32_t buffers_count)
{
	UNUSED(buffers_count);
	return 0;
}

int32_t M2kLogicRX::get_mask(uint32_t* mask)
{
	UNUSED(mask);
	return 0;
}

ssize_t M2kLogicRX::read_dev(char* pbuf, size_t offset, size_t bytes_count)
{
	UNUSED(offset);
	std::vector<uint16_t> samples = resample(0, bytes_count);

	memcpy(pbuf, samples.data(), bytes_count);
	return static_cast<ssize_t>(bytes_count);
}

void M2kLogicRX::connectDevice(unsigned short channel_in, AbstractDeviceOut* deviceOut, unsigned short channel_out)
{
	m_connections.at(channel_in) = std::pair<AbstractDeviceOut*, unsigned short>(deviceOut, channel_out);
}

std::vector<uint16_t> M2kLogicRX::resample(unsigned short channel, size_t len)
{
	UNUSED(channel);
	loadValues();
	std::vector<uint16_t> samples;
	auto ratio = static_cast<unsigned int>(1E8 / m_samplerate);

	std::vector<uint16_t> tmp_samples((len / 2) * ratio);
	m_connections.at(0).first->transfer_samples_to_RX_device(reinterpret_cast<char*>(tmp_samples.data()),
								 (len / 2) * ratio);

	digital_decimation(tmp_samples, samples, ratio);

	return samples;
}

void M2kLogicRX::loadValues()
{
	char tmp_attr[IIOD_BUFFER_SIZE];

	read_device_attr(m_doc, m_device_id, "sampling_frequency", tmp_attr, IIOD_BUFFER_SIZE, IIO_ATTR_TYPE_DEVICE);
	m_samplerate = safe_stod(tmp_attr);
}

int32_t M2kLogicRX::cancel_buffer() { return 0; }

ssize_t M2kLogicRX::transfer_dev_to_mem(size_t bytes_count)
{
	UNUSED(bytes_count);
	return 0;
}
