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

#include "tinyiiod_ops_wrapper.hpp"

#include "abstract_ops.hpp"
#include "utils/logger.hpp"

using namespace iio_emu;

AbstractOps* g_ops;

void iio_emu::set_ops(iio_emu::AbstractOps* ops) { g_ops = ops; }

ssize_t iio_emu::read(char* buf, size_t len)
{
	if (g_ops == nullptr) {
		return -ENOENT;
	}
	Logger::log(IIO_EMU_DEBUG, {"Tinyiiod read"});
	return g_ops->readData(buf, len);
}

ssize_t iio_emu::write(const char* buf, size_t len)
{
	if (g_ops == nullptr) {
		return -ENOENT;
	}
	Logger::log(IIO_EMU_DEBUG, {"Tinyiiod write"});
	return g_ops->writeData(buf, len);
}

ssize_t iio_emu::read_line(char* buf, size_t len)
{
	if (g_ops == nullptr) {
		return -ENOENT;
	}
	Logger::log(IIO_EMU_DEBUG, {"Tinyiiod readline"});
	return g_ops->readLine(buf, len);
}

ssize_t iio_emu::open_instance()
{
	if (g_ops == nullptr) {
		return -ENOENT;
	}
	Logger::log(IIO_EMU_DEBUG, {"Tinyiiod open_instance"});
	return g_ops->openInstance();
}

ssize_t iio_emu::close_instance()
{
	if (g_ops == nullptr) {
		return -ENOENT;
	}
	Logger::log(IIO_EMU_DEBUG, {"Tinyiiod close_instance"});
	return g_ops->closeInstance();
}

ssize_t iio_emu::read_attr(const char* device_id, const char* attr, char* buf, size_t len, enum iio_attr_type type)
{
	if (g_ops == nullptr) {
		return -ENOENT;
	}
	Logger::log(IIO_EMU_DEBUG, {"Tinyiiod read_attr: ", attr});
	return g_ops->readAttr(device_id, attr, buf, len, type);
}

ssize_t iio_emu::write_attr(const char* device_id, const char* attr, const char* buf, size_t len,
			    enum iio_attr_type type)
{
	if (g_ops == nullptr) {
		return -ENOENT;
	}
	Logger::log(IIO_EMU_DEBUG, {"Tinyiiod write_attr: ", attr});
	return g_ops->writeAttr(device_id, attr, buf, len, type);
}

ssize_t iio_emu::ch_read_attr(const char* device_id, const char* channel, bool ch_out, const char* attr, char* buf,
			      size_t len)
{
	if (g_ops == nullptr) {
		return -ENOENT;
	}
	Logger::log(IIO_EMU_DEBUG, {"Tinyiiod ch_read_attr: ", attr});
	return g_ops->chReadAttr(device_id, channel, ch_out, attr, buf, len);
}

ssize_t iio_emu::ch_write_attr(const char* device_id, const char* channel, bool ch_out, const char* attr,
			       const char* buf, size_t len)
{
	if (g_ops == nullptr) {
		return -ENOENT;
	}
	Logger::log(IIO_EMU_DEBUG, {"Tinyiiod ch_write_attr: ", attr});
	return g_ops->chWriteAttr(device_id, channel, ch_out, attr, buf, len);
}

int32_t iio_emu::open(const char* device, size_t sample_size, uint32_t mask, bool cyclic)
{
	if (g_ops == nullptr) {
		return -ENOENT;
	}
	Logger::log(IIO_EMU_DEBUG, {"Tinyiiod open"});
	return g_ops->openDev(device, sample_size, mask, cyclic);
}

int32_t iio_emu::close(const char* device)
{
	if (g_ops == nullptr) {
		return -ENOENT;
	}
	Logger::log(IIO_EMU_DEBUG, {"Tinyiiod close"});
	return g_ops->closeDev(device);
}

ssize_t iio_emu::transfer_dev_to_mem(const char* device, size_t bytes_count)
{
	if (g_ops == nullptr) {
		return -ENOENT;
	}
	Logger::log(IIO_EMU_DEBUG, {"Tinyiiod transfer_dev_to_mem: ", std::to_string(bytes_count)});
	return g_ops->transferDevToMem(device, bytes_count);
}

ssize_t iio_emu::read_data(const char* device, char* pbuf, size_t offset, size_t bytes_count)
{
	if (g_ops == nullptr) {
		return -ENOENT;
	}
	Logger::log(IIO_EMU_DEBUG, {"Tinyiiod read_data: ", std::to_string(bytes_count)});
	return g_ops->readDev(device, pbuf, offset, bytes_count);
}

ssize_t iio_emu::transfer_mem_to_dev(const char* device, size_t bytes_count)
{
	if (g_ops == nullptr) {
		return -ENOENT;
	}
	Logger::log(IIO_EMU_DEBUG, {"Tinyiiod transfer_mem_to_dev: ", std::to_string(bytes_count)});
	return g_ops->transferMemToDev(device, bytes_count);
}

ssize_t iio_emu::write_data(const char* device, const char* buf, size_t offset, size_t bytes_count)
{
	if (g_ops == nullptr) {
		return -ENOENT;
	}
	Logger::log(IIO_EMU_DEBUG, {"Tinyiiod write_data: ", std::to_string(bytes_count)});
	return g_ops->writeDev(device, buf, offset, bytes_count);
}

int32_t iio_emu::get_mask(const char* device, uint32_t* mask)
{
	if (g_ops == nullptr) {
		return -ENOENT;
	}
	Logger::log(IIO_EMU_DEBUG, {"Tinyiiod get_mask"});
	return g_ops->getMask(device, mask);
}

int32_t iio_emu::set_timeout(uint32_t timeout)
{
	if (g_ops == nullptr) {
		return -ENOENT;
	}
	Logger::log(IIO_EMU_DEBUG, {"Tinyiiod set_timeout: ", std::to_string(timeout)});
	return g_ops->setTimeout(timeout);
}

int32_t iio_emu::set_buffers_count(const char* device, uint32_t buffers_count)
{
	if (g_ops == nullptr) {
		return -ENOENT;
	}
	Logger::log(IIO_EMU_DEBUG, {"Tinyiiod set_buffers_count: ", std::to_string(buffers_count)});
	return g_ops->setBuffersCount(device, buffers_count);
}

ssize_t iio_emu::get_xml(char** outxml)
{
	if (g_ops == nullptr) {
		return -ENOENT;
	}
	Logger::log(IIO_EMU_DEBUG, {"Tinyiiod get_xml"});
	return g_ops->getXml(outxml);
}
