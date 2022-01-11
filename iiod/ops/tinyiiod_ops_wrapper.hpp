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

#ifndef IIO_EMU_TINYIIOD_OPS_WRAPPER_HPP
#define IIO_EMU_TINYIIOD_OPS_WRAPPER_HPP

#include <tinyiiod/tinyiiod.h>

namespace iio_emu {

class AbstractOps;

void set_ops(iio_emu::AbstractOps* ops);

ssize_t read(char* buf, size_t len);
ssize_t write(const char* buf, size_t len);
ssize_t read_line(char* buf, size_t len);

ssize_t open_instance();
ssize_t close_instance();

ssize_t read_attr(const char* device_id, const char* attr, char* buf, size_t len, enum iio_attr_type type);
ssize_t write_attr(const char* device_id, const char* attr, const char* buf, size_t len, enum iio_attr_type type);

ssize_t ch_read_attr(const char* device_id, const char* channel, bool ch_out, const char* attr, char* buf, size_t len);
ssize_t ch_write_attr(const char* device_id, const char* channel, bool ch_out, const char* attr, const char* buf,
		      size_t len);

int32_t open(const char* device, size_t sample_size, uint32_t mask, bool cyclic);
int32_t close(const char* device);

ssize_t transfer_dev_to_mem(const char* device, size_t bytes_count);
ssize_t read_data(const char* device, char* pbuf, size_t offset, size_t bytes_count);

ssize_t transfer_mem_to_dev(const char* device, size_t bytes_count);
ssize_t write_data(const char* device, const char* buf, size_t offset, size_t bytes_count);

int32_t get_mask(const char* device, uint32_t* mask);

int32_t set_timeout(uint32_t timeout);

int32_t get_trigger(const char* device, char* trigger, size_t len);
int32_t set_trigger(const char* device, const char* trigger, size_t len);

int32_t set_buffers_count(const char* device, uint32_t buffers_count);

ssize_t get_xml(char** outxml);

} // namespace iio_emu
#endif // IIO_EMU_TINYIIOD_OPS_WRAPPER_HPP
