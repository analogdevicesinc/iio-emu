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

#include "m2k_adc.hpp"

#include "utils/attr_ops_xml.hpp"
#include "utils/utility.hpp"

#include <thread>

#define M2K_ADC_CHANNELS 2
#define M2K_ADC_SAMPLE_SIZE 2
#define M2K_ADC_CHANNEL_1 0
#define M2K_ADC_CHANNEL_2 1

using namespace iio_emu;

M2kADC::M2kADC(const char* device_id, struct _xmlDoc* doc)
{
	m_device_id = device_id;
	m_doc = doc;

	m_connections = std::vector<std::pair<AbstractDeviceOut*, unsigned short>>(M2K_ADC_CHANNELS);

	m_range.push_back(new char[IIOD_BUFFER_SIZE]());
	m_range.push_back(new char[IIOD_BUFFER_SIZE]());
	m_hw_offset = std::vector<double>(M2K_ADC_CHANNELS);

	m_filter_compensation_table[1E8] = 1.00;
	m_filter_compensation_table[1E7] = 1.05;
	m_filter_compensation_table[1E6] = 1.10;
	m_filter_compensation_table[1E5] = 1.15;
	m_filter_compensation_table[1E4] = 1.20;
	m_filter_compensation_table[1E3] = 1.26;
}

M2kADC::~M2kADC()
{
	for (auto range : m_range) {
		delete range;
	}
}

int32_t M2kADC::open_dev(size_t sample_size, uint32_t mask, bool cyclic)
{
	UNUSED(sample_size);
	UNUSED(mask);
	UNUSED(cyclic);
	return 0;
}

int32_t M2kADC::close_dev() { return 0; }

int32_t M2kADC::set_buffers_count(uint32_t buffers_count)
{
	UNUSED(buffers_count);
	return 0;
}

int32_t M2kADC::get_mask(uint32_t* mask)
{
	*mask = 3;
	return 0;
}

ssize_t M2kADC::read_dev(char* pbuf, size_t offset, size_t bytes_count)
{
	UNUSED(offset);
	loadCalibValues();

	std::vector<int16_t> samples;
	samples.reserve(bytes_count / M2K_ADC_SAMPLE_SIZE);
	auto ratio = static_cast<unsigned int>(1E8 / (m_samplerate / m_oversampling_ratio));
	size_t size = (bytes_count / (M2K_ADC_CHANNELS * M2K_ADC_SAMPLE_SIZE)) * ratio;

	std::vector<double> dac_a_samples;
	std::vector<double> dac_b_samples;

	std::thread read_ch1(M2kADC::resample, m_connections.at(M2K_ADC_CHANNEL_1).first, size, ratio,
			     std::ref(dac_a_samples));
	std::thread read_ch2(M2kADC::resample, m_connections.at(M2K_ADC_CHANNEL_2).first, size, ratio,
			     std::ref(dac_b_samples));

	read_ch1.join();
	read_ch2.join();

	for (size_t i = 0, j = 0; i < bytes_count / (M2K_ADC_CHANNELS * M2K_ADC_SAMPLE_SIZE); i++, j += 2) {
		samples.emplace_back(convertVoltToRaw(dac_a_samples.at(i), M2K_ADC_CHANNEL_1));
		samples.emplace_back(convertVoltToRaw(dac_b_samples.at(i), M2K_ADC_CHANNEL_2));
	}

	memcpy(pbuf, samples.data(), bytes_count);
	return static_cast<ssize_t>(bytes_count);
}

void M2kADC::connectDevice(unsigned short channel_in, AbstractDeviceOut* deviceOut, unsigned short channel_out)
{
	m_connections.at(channel_in) = std::pair<AbstractDeviceOut*, unsigned short>(deviceOut, channel_out);
}

int16_t M2kADC::convertVoltToRaw(double voltage, unsigned short channel) const
{
	const double correctionGain = 1;
	const double hw_gain = getCalibGain(channel);
	const double filterCompensation = getFilterCompensation();
	const double offset = -m_hw_offset.at(channel);

	return static_cast<int16_t>((voltage - offset) / (correctionGain * filterCompensation) *
				    (2048 * 1.3 * hw_gain) / 0.78);
}

double M2kADC::convertRawToVoltsVerticalOffset(int16_t raw, unsigned short channel) const
{
	const double gain = 1.3;
	const double vref = 1.2;
	const double hw_range_gain = getCalibGain(channel);

	return raw * 2.693 * vref / ((1u << 12u) * hw_range_gain * gain);
}

double M2kADC::getCalibGain(unsigned short channel) const
{
	if (!strncmp(m_range.at(channel), "high", sizeof("high") - 1)) {
		return 0.21229;
	} else if (!strncmp(m_range.at(channel), "low", sizeof("low") - 1)) {
		return 0.02017;
	} else {
		return 0;
	}
}

double M2kADC::getFilterCompensation() const
{
	double compensation = 0.0;
	if (m_filter_compensation_table.find(m_samplerate) != m_filter_compensation_table.end()) {
		compensation = m_filter_compensation_table.at(m_samplerate);
	}

	return compensation;
}

void M2kADC::loadCalibValues()
{
	char tmp_attr[IIOD_BUFFER_SIZE];

	read_channel_attr(m_doc, "iio:device11", "voltage0", false, "gain", m_range.at(M2K_ADC_CHANNEL_1),
			  IIOD_BUFFER_SIZE);
	read_channel_attr(m_doc, "iio:device11", "voltage1", false, "gain", m_range.at(M2K_ADC_CHANNEL_2),
			  IIOD_BUFFER_SIZE);

	read_channel_attr(m_doc, "iio:device2", "voltage2", true, "raw", tmp_attr, IIOD_BUFFER_SIZE);
	m_hw_offset.at(M2K_ADC_CHANNEL_1) = safe_stod(tmp_attr);
	read_channel_attr(m_doc, "iio:device0", "voltage0", false, "calibbias", tmp_attr, IIOD_BUFFER_SIZE);
	m_hw_offset.at(M2K_ADC_CHANNEL_1) -= safe_stod(tmp_attr);
	m_hw_offset.at(M2K_ADC_CHANNEL_1) = convertRawToVoltsVerticalOffset(
		static_cast<int16_t>(m_hw_offset.at(M2K_ADC_CHANNEL_1)), M2K_ADC_CHANNEL_1);

	read_channel_attr(m_doc, "iio:device2", "voltage3", true, "raw", tmp_attr, IIOD_BUFFER_SIZE);
	m_hw_offset.at(M2K_ADC_CHANNEL_2) = safe_stod(tmp_attr);
	read_channel_attr(m_doc, "iio:device0", "voltage1", false, "calibbias", tmp_attr, IIOD_BUFFER_SIZE);
	m_hw_offset.at(M2K_ADC_CHANNEL_2) -= safe_stod(tmp_attr);
	m_hw_offset.at(M2K_ADC_CHANNEL_2) = convertRawToVoltsVerticalOffset(
		static_cast<int16_t>(m_hw_offset.at(M2K_ADC_CHANNEL_2)), M2K_ADC_CHANNEL_2);

	read_device_attr(m_doc, "iio:device0", "sampling_frequency", tmp_attr, IIOD_BUFFER_SIZE, IIO_ATTR_TYPE_DEVICE);
	m_samplerate = safe_stod(tmp_attr);

	read_device_attr(m_doc, "iio:device0", "oversampling_ratio", tmp_attr, IIOD_BUFFER_SIZE, IIO_ATTR_TYPE_DEVICE);
	m_oversampling_ratio = static_cast<unsigned int>(std::stoi(tmp_attr));
}

void M2kADC::resample(AbstractDeviceOut* devOut, size_t size, unsigned int ratio, std::vector<double>& dest)
{
	std::vector<double> tmp_samples(size);

	devOut->transfer_samples_to_RX_device(reinterpret_cast<char*>(tmp_samples.data()), size);

	if (ratio < 2) {
		dest = tmp_samples;
		return;
	}

	analogical_decimation(tmp_samples, dest, ratio);
}

int32_t M2kADC::cancel_buffer() { return 0; }

ssize_t M2kADC::transfer_dev_to_mem(size_t bytes_count)
{
	UNUSED(bytes_count);
	return 0;
}
