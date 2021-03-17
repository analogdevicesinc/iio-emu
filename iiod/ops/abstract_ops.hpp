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

#ifndef IIO_EMU_ABSTRACT_OPS_HPP
#define IIO_EMU_ABSTRACT_OPS_HPP

extern "C"
{
#include <tinyiiod/tinyiiod.h>
}

namespace iio_emu {

class AbstractSocket;

class AbstractOps
{
public:
	AbstractOps();
	virtual ~AbstractOps() = default;

	virtual struct tinyiiod_ops* getIIODOps();

	virtual void setCurrentSocket(AbstractSocket* socket) = 0;
	virtual void socketDisconnected(int fd) = 0;

	// tinyiiod ops:
	// communication
	virtual ssize_t readData(char* buf, size_t len) = 0;
	virtual ssize_t writeData(const char* buf, size_t len) = 0;
	virtual ssize_t readLine(char* buf, size_t len) = 0;

	// open/close iiod instance
	virtual ssize_t openInstance() = 0;
	virtual ssize_t closeInstance() = 0;

	// device, debug, buffer attributes
	virtual ssize_t readAttr(const char* device_id, const char* attr, char* buf, size_t len,
				 enum iio_attr_type type) = 0;
	virtual ssize_t writeAttr(const char* device_id, const char* attr, const char* buf, size_t len,
				  enum iio_attr_type type) = 0;

	// channel attributes
	virtual ssize_t chReadAttr(const char* device_id, const char* channel, bool ch_out, const char* attr, char* buf,
				   size_t len) = 0;
	virtual ssize_t chWriteAttr(const char* device_id, const char* channel, bool ch_out, const char* attr,
				    const char* buf, size_t len) = 0;

	// open/close device
	virtual int32_t openDev(const char* device, size_t sample_size, uint32_t mask, bool cyclic) = 0;
	virtual int32_t closeDev(const char* device) = 0;

	// read device buffer
	virtual ssize_t transferDevToMem(const char* device,
					 size_t bytes_count) = 0; // called at the end of transmission
	virtual ssize_t readDev(const char* device, char* pbuf, size_t offset, size_t bytes_count) = 0;

	// write device buffer
	virtual ssize_t transferMemToDev(const char* device,
					 size_t bytes_count) = 0; // called at the end of transmission
	virtual ssize_t writeDev(const char* device, const char* buf, size_t offset, size_t bytes_count) = 0;

	// the mask maps the enabled channels
	virtual int32_t getMask(const char* device, uint32_t* mask) = 0;

	virtual int32_t setTimeout(uint32_t timeout) = 0;

	virtual int32_t setBuffersCount(const char* device, uint32_t buffers_count) = 0;

	virtual ssize_t getXml(char** outxml) = 0;

protected:
	struct tinyiiod_ops* m_iiodOps;
};
} // namespace iio_emu
#endif // IIO_EMU_ABSTRACT_OPS_HPP
