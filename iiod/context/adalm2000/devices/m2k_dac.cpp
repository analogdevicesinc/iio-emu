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

#include "m2k_dac.hpp"

#include "utils/attr_ops_xml.hpp"
#include "utils/utility.hpp"

#include <vector>

using namespace iio_emu;

M2kDAC::M2kDAC(const char* device_id, struct _xmlDoc* doc)
{
	m_device_id = device_id;
	m_doc = doc;
	m_reset_buffer = true;

	m_current_index = 0;

	m_calib_vlsb = 10.0 / ((1 << 12) - 1);

	m_filter_compensation_table[75E6] = 1.00;
	m_filter_compensation_table[75E5] = 1.525879;
	m_filter_compensation_table[75E4] = 1.164153;
	m_filter_compensation_table[75E3] = 1.776357;
	m_filter_compensation_table[75E2] = 1.355253;
	m_filter_compensation_table[75E1] = 1.033976;
}

M2kDAC::~M2kDAC() {}

int32_t M2kDAC::open_dev(size_t sample_size, uint32_t mask, bool cyclic)
{
	UNUSED(sample_size);
	loadCalibValues();
	m_cyclic = cyclic;
	m_enable = false;
	if (mask) {
		m_enable = true;
	}
	return 0;
}

int32_t M2kDAC::close_dev()
{
	m_samples.clear();
	return 0;
}

int32_t M2kDAC::set_buffers_count(uint32_t buffers_count)
{
	UNUSED(buffers_count);
	return 0;
}

int32_t M2kDAC::get_mask(uint32_t* mask)
{
	*mask = 0;
	if (m_enable) {
		*mask = 1;
	}
	return 0;
}

ssize_t M2kDAC::write_dev(const char* buf, size_t offset, size_t bytes_count)
{
	UNUSED(offset);
	if (!m_enable) {
		return 0;
	}

	if (m_reset_buffer) {
		m_samples.clear();
		m_current_index = 0;
		m_reset_buffer = false;
	}

	auto* samples = reinterpret_cast<const int16_t*>(buf);
	for (size_t i = 0; i < bytes_count / 2; i++) {
		m_samples.push_back(convertRawToVolts(samples[i]));
	}

	return static_cast<ssize_t>(bytes_count);
}

double M2kDAC::convertRawToVolts(int16_t raw) const
{
	const double filterCompensation = getFilterCompensation();
	return -(((raw >> 4) + 0.5) * filterCompensation * m_calib_vlsb);
}

double M2kDAC::getFilterCompensation() const { return m_filter_compensation_table.at(m_samplerate); }

void M2kDAC::loadCalibValues()
{
	char tmp_attr[IIOD_BUFFER_SIZE];

	read_device_attr(m_doc, m_device_id, "sampling_frequency", tmp_attr, IIOD_BUFFER_SIZE, IIO_ATTR_TYPE_DEVICE);
	m_samplerate = safe_stod(tmp_attr);

	read_device_attr(m_doc, m_device_id, "oversampling_ratio", tmp_attr, IIOD_BUFFER_SIZE, IIO_ATTR_TYPE_DEVICE);
	m_oversampling_ratio = static_cast<unsigned int>(std::stoi(tmp_attr));
}

std::vector<double> M2kDAC::resample()
{
	loadCalibValues();
	auto ratio = static_cast<unsigned int>(75E6 / (m_samplerate / m_oversampling_ratio));

	if (ratio < 2) {
		return m_samples;
	}
	std::vector<double> samples;

	analogical_interpolation(m_samples, samples, ratio);

	return samples;
}

ssize_t M2kDAC::transfer_mem_to_dev(size_t bytes_count)
{
	UNUSED(bytes_count);
	m_samples = resample();
	m_reset_buffer = true;
	return 0;
}

void M2kDAC::transfer_samples_to_RX_device(char* buf, size_t samples_count)
{
	if (m_samples.empty()) {
		m_samples = std::vector<double>(samples_count);
	}

	size_t buffer_index = 0;
	size_t remaining_samples;
	while (samples_count > 0) {
		remaining_samples = m_samples.size() - m_current_index;
		remaining_samples = std::min(remaining_samples, samples_count);
		memcpy(buf + (buffer_index * sizeof(double)), m_samples.data() + m_current_index,
		       remaining_samples * sizeof(double));
		buffer_index += remaining_samples;
		samples_count -= remaining_samples;
		m_current_index = (m_current_index + static_cast<unsigned int>(remaining_samples)) %
			static_cast<unsigned int>(m_samples.size());
	}
}

int32_t M2kDAC::cancel_buffer()
{
	m_samples.clear();
	m_current_index = 0;
	return 0;
}
