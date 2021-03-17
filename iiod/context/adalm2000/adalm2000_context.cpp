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

#include "adalm2000_context.hpp"

#include "iiod/context/adalm2000/devices/m2k_adc.hpp"
#include "iiod/context/adalm2000/devices/m2k_dac.hpp"
#include "iiod/context/adalm2000/devices/m2k_logic_rx.hpp"
#include "iiod/context/adalm2000/devices/m2k_logic_tx.hpp"
#include "utils/attr_ops_xml.hpp"
#include "utils/utility.hpp"

#include <adalm2000_xml.h>
#include <libxml/tree.h>

using namespace iio_emu;

enum CALIBRATION_COEFFICIENT
{
	DAC_OFFSET_POS = 0,
	DAC_GAIN_POS = 1,
	ADC_OFFSET_POS = 2,
	ADC_GAIN_POS = 3,
	DAC_OFFSET_NEG = 4,
	DAC_GAIN_NEG = 5,
	ADC_OFFSET_NEG = 6,
	ADC_GAIN_NEG = 7
};

Adalm2000Context::Adalm2000Context()
	: GenericXmlContext(reinterpret_cast<const char*>(adalm2000_xml), sizeof(adalm2000_xml))
{
	// devices
	auto adc = new M2kADC("iio:device0", m_doc);
	auto dac_a = new M2kDAC("iio:device6", m_doc);
	auto dac_b = new M2kDAC("iio:device7", m_doc);
	auto logic_rx = new M2kLogicRX("iio:device10", m_doc);
	auto logic_tx = new M2kLogicTX("iio:device9", m_doc);

	addDevice(adc);
	addDevice(dac_a);
	addDevice(dac_b);
	addDevice(logic_rx);
	addDevice(logic_tx);

	// connections
	adc->connectDevice(0, dac_a, 0);
	adc->connectDevice(1, dac_b, 0);
	logic_rx->connectDevice(0, logic_tx, 0);

	assignBasicOps();

	loadPSCalibCoef();

	m_ps_write_coefficients.push_back(4095.0 / (5.02 * 1.2));
	m_ps_write_coefficients.push_back(4095.0 / (-5.1 * 1.2));
	m_ps_read_coefficients.push_back(6.4 / 4095.0);
	m_ps_read_coefficients.push_back((-6.4) / 4095.0);

	m_ps_current_values = std::vector<std::string>(2);
}

Adalm2000Context::~Adalm2000Context()
{
	delete m_iiodOps;
	m_iiodOps = nullptr;

	xmlFreeDoc(m_doc);
	xmlCleanupParser();
	m_doc = nullptr;

	delete m_ctx_xml;
	m_ctx_xml = nullptr;

	for (auto dev : m_devices) {
		delete dev;
		dev = nullptr;
	}
}

ssize_t Adalm2000Context::chWriteAttr(const char* device_id, const char* channel, bool ch_out, const char* attr,
				      const char* buf, size_t len)
{
	auto ret = GenericXmlContext::chWriteAttr(device_id, channel, ch_out, attr, buf, len);
	if (!strncmp(device_id, "iio:device3", sizeof("iio:device3") - 1)) {

		std::string channelRead;
		unsigned short channelIdx;

		if (!strncmp(channel, "voltage0", sizeof("voltage0") - 1)) {
			channelIdx = 0;
			channelRead = "voltage2";
		} else {
			channelIdx = 1;
			channelRead = "voltage1";
		}

		if (!strncmp(attr, "raw", sizeof("raw") - 1)) {
			auto voltage = convertPSDACRawToVolts(channelIdx, std::stoi(buf));
			auto result = convertPSADCVoltsToRaw(channelIdx, voltage);
			m_ps_current_values.at(channelIdx) = std::to_string(result);
		}

		char buffer[IIOD_BUFFER_SIZE];
		read_channel_attr(m_doc, device_id, channel, ch_out, "powerdown", buffer, IIOD_BUFFER_SIZE);

		if (!strncmp(buffer, "1", 1)) {
			GenericXmlContext::chWriteAttr("iio:device13", channelRead.c_str(), false, "raw", "0", 1);
		} else {
			GenericXmlContext::chWriteAttr("iio:device13", channelRead.c_str(), false, "raw",
						       m_ps_current_values.at(channelIdx).c_str(),
						       m_ps_current_values.at(channelIdx).size());
		}
	}
	return ret;
}

void Adalm2000Context::loadPSCalibCoef()
{
	m_ps_calib_coefficients.clear();
	m_ps_calib_coefficients.reserve(8);

	char buf[IIOD_BUFFER_SIZE];
	std::vector<std::string> calibAttrs = {"cal,offset_pos_dac", "cal,gain_pos_dac",   "cal,offset_pos_adc",
					       "cal,gain_pos_adc",   "cal,offset_neg_dac", "cal,gain_neg_dac",
					       "cal,offset_neg_adc", "cal,gain_neg_adc"};

	for (auto& attrName : calibAttrs) {
		read_context_attr(m_doc, attrName.c_str(), buf, IIOD_BUFFER_SIZE);
		m_ps_calib_coefficients.push_back(safe_stod(buf));
	}
}

double Adalm2000Context::convertPSDACRawToVolts(unsigned short channel, int value) const
{
	double offset, gain;

	if (channel == 0) {
		offset = m_ps_calib_coefficients.at(DAC_OFFSET_POS);
		gain = m_ps_calib_coefficients.at(DAC_GAIN_POS);
	} else {
		offset = m_ps_calib_coefficients.at(DAC_OFFSET_NEG);
		gain = m_ps_calib_coefficients.at(DAC_GAIN_NEG);
	}

	return ((value / gain) - offset) / m_ps_write_coefficients.at(channel);
}

int Adalm2000Context::convertPSADCVoltsToRaw(unsigned short channel, double value) const
{
	double offset, gain;

	if (channel == 0) {
		offset = m_ps_calib_coefficients.at(ADC_OFFSET_POS);
		gain = m_ps_calib_coefficients.at(ADC_GAIN_POS);
	} else {
		offset = m_ps_calib_coefficients.at(ADC_OFFSET_NEG);
		gain = m_ps_calib_coefficients.at(ADC_GAIN_NEG);
	}

	return static_cast<int>(((value / gain) - offset) / m_ps_read_coefficients.at(channel));
}
