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

#ifndef IIO_EMU_M2K_LOGIC_RX_HPP
#define IIO_EMU_M2K_LOGIC_RX_HPP

#include "iiod/devices/abstract_device_in.hpp"

struct _xmlDoc;

namespace iio_emu {

class M2kLogicRX : public AbstractDeviceIn
{
public:
	M2kLogicRX(const char* device_id, struct _xmlDoc* doc);
	~M2kLogicRX() override;

	int32_t open_dev(size_t sample_size, uint32_t mask, bool cyclic) override;
	int32_t close_dev() override;
	int32_t set_buffers_count(uint32_t buffers_count) override;
	int32_t get_mask(uint32_t* mask) override;
	ssize_t read_dev(char* pbuf, size_t offset, size_t bytes_count) override;

	int32_t cancel_buffer() override;

	ssize_t transfer_dev_to_mem(size_t bytes_count) override;
	void connectDevice(unsigned short channel_in, AbstractDeviceOut* deviceOut,
			   unsigned short channel_out) override;

private:
	struct _xmlDoc* m_doc;
	std::vector<std::pair<AbstractDeviceOut*, unsigned short>> m_connections;

	double m_samplerate;
	void loadValues();
	std::vector<uint16_t> resample(unsigned short channel, size_t len);
};
} // namespace iio_emu
#endif // IIO_EMU_M2K_LOGIC_RX_HPP
