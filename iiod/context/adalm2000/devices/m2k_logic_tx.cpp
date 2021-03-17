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

#include "m2k_logic_tx.hpp"

#include "utils/attr_ops_xml.hpp"
#include "utils/utility.hpp"

using namespace iio_emu;

M2kLogicTX::M2kLogicTX(const char* device_id, struct _xmlDoc* doc)
{
	m_device_id = device_id;
	m_doc = doc;
	m_current_index = 0;

	m_reset_buffer = false;
}

M2kLogicTX::~M2kLogicTX() {}

int32_t M2kLogicTX::open_dev(size_t sample_size, uint32_t mask, bool cyclic)
{
	UNUSED(sample_size);
	UNUSED(mask);
	UNUSED(cyclic);
	return 0;
}

int32_t M2kLogicTX::close_dev() { return 0; }

int32_t M2kLogicTX::set_buffers_count(uint32_t buffers_count)
{
	UNUSED(buffers_count);
	return 0;
}

int32_t M2kLogicTX::get_mask(uint32_t* mask)
{
	UNUSED(mask);
	return 0;
}

ssize_t M2kLogicTX::write_dev(const char* buf, size_t offset, size_t bytes_count)
{
	UNUSED(offset);
	if (m_reset_buffer) {
		m_samples.clear();
		m_current_index = 0;
		m_reset_buffer = false;
	}

	auto* samples = reinterpret_cast<const uint16_t*>(buf);
	for (size_t i = 0; i < bytes_count / 2; i++) {
		m_samples.push_back(samples[i]);
	}

	return static_cast<ssize_t>(bytes_count);
}

ssize_t M2kLogicTX::transfer_mem_to_dev(size_t bytes_count)
{
	UNUSED(bytes_count);
	m_samples = resample();
	m_reset_buffer = true;
	return 0;
}

std::vector<uint16_t> M2kLogicTX::resample()
{
	loadValues();
	auto ratio = static_cast<unsigned int>(1E8 / m_samplerate);

	if (ratio < 2) {
		return m_samples;
	}

	std::vector<uint16_t> samples;
	digital_interpolation(m_samples, samples, ratio);

	return samples;
}

void M2kLogicTX::loadValues()
{
	char tmp_attr[IIOD_BUFFER_SIZE];

	read_device_attr(m_doc, m_device_id, "sampling_frequency", tmp_attr, IIOD_BUFFER_SIZE, IIO_ATTR_TYPE_DEVICE);
	m_samplerate = safe_stod(tmp_attr);
}

void M2kLogicTX::transfer_samples_to_RX_device(char* buf, size_t samples_count)
{
	if (m_samples.empty()) {
		m_samples = std::vector<uint16_t>(samples_count);
	}

	std::vector<uint16_t> samples;

	for (size_t i = 0; i < samples_count; i++) {
		samples.push_back(m_samples.at(m_current_index));
		m_current_index++;
		if (m_current_index >= m_samples.size()) {
			m_current_index = 0;
		}
	}
	memcpy(buf, samples.data(), samples_count * sizeof(uint16_t));
}

int32_t M2kLogicTX::cancel_buffer()
{
	m_samples.clear();
	return 0;
}
