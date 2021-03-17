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

#ifndef IIO_EMU_M2K_DAC_HPP
#define IIO_EMU_M2K_DAC_HPP

#include "iiod/devices/abstract_device_out.hpp"
#include <vector>
#include <map>

struct _xmlDoc;

namespace iio_emu {

class M2kDAC : public AbstractDeviceOut {
public:
	M2kDAC(const char *device_id, struct _xmlDoc *doc);
	~M2kDAC() override;

	int32_t open_dev(size_t sample_size, uint32_t mask, bool cyclic) override;
	int32_t close_dev() override;
	int32_t set_buffers_count(uint32_t buffers_count) override;
	int32_t get_mask(uint32_t *mask) override;
	ssize_t write_dev(const char *buf, size_t offset, size_t bytes_count) override;
	ssize_t transfer_mem_to_dev(size_t bytes_count) override;

	int32_t cancel_buffer() override;

	void transfer_samples_to_RX_device(char *buf, size_t samples_count) override;

private:
	struct _xmlDoc *m_doc;
	bool m_cyclic;
	bool m_enable;


	unsigned int m_current_index;

	double m_samplerate;
	unsigned int m_oversampling_ratio;
	double m_calib_vlsb;
	std::map<double, double> m_filter_compensation_table;
	std::vector<double> m_samples;
	bool m_reset_buffer;


	double convertRawToVolts(int16_t raw) const;
	double getFilterCompensation() const;
	void loadCalibValues();
	std::vector<double> resample();
};
}

#endif //IIO_EMU_M2K_DAC_HPP
