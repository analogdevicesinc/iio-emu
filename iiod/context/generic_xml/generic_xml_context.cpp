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

#include "generic_xml_context.hpp"

#include "iiod/devices/abstract_device.hpp"
#include "iiod/devices/abstract_device_in.hpp"
#include "iiod/devices/abstract_device_out.hpp"
#include "iiod/ops/tinyiiod_ops_wrapper.hpp"
#include "networking/abstract_socket.hpp"
#include "utils/attr_ops_xml.hpp"
#include "utils/input_parser.hpp"
#include "utils/network_ops.hpp"
#include "utils/utility.hpp"

#include <iiod/context/generic_xml/devices/generic_rx_device.hpp>
#include <iiod/context/generic_xml/devices/generic_tx_device.hpp>
#include <libxml/tree.h>

using namespace iio_emu;

GenericXmlContext::GenericXmlContext(std::vector<const char*>& args)
{
	auto xmlPath = InputParser::getXMLPath(args);
	auto devices = InputParser::getDevices(args);

	// TODO: check xmlPath
	m_doc = xmlReadFile(xmlPath, nullptr, XML_PARSE_DTDVALID);
	m_xml_size = iio_emu::getXml(m_doc, &m_ctx_xml);

	for (const auto& devInfo : devices) {
		if (isScanChannel(devInfo.first.c_str())) {
			AbstractDevice* dev;
			if (isOutputChannel(devInfo.first.c_str())) {
				dev = new GenericTXDevice(devInfo.first.c_str(), devInfo.second.c_str());
			} else {
				dev = new GenericRXDevice(devInfo.first.c_str(), devInfo.second.c_str());
			}
			addDevice(dev);
		}
	}

	assignBasicOps();
}

GenericXmlContext::GenericXmlContext(const char* file, int fileSize)
{
	m_doc = xmlReadMemory(file, fileSize, nullptr, nullptr, XML_PARSE_DTDVALID);
	m_xml_size = iio_emu::getXml(m_doc, &m_ctx_xml);
}

GenericXmlContext::~GenericXmlContext()
{
	delete m_iiodOps;
	m_iiodOps = nullptr;

	xmlFreeDoc(m_doc);
	xmlCleanupParser();
	m_doc = nullptr;

	delete m_ctx_xml;
	m_ctx_xml = nullptr;

	for (auto dev : m_devices) {
		if (dev) {
			delete dev;
			dev = nullptr;
		}
	}
}

AbstractDevice* GenericXmlContext::getDevice(const char* device_id) const
{
	AbstractDevice* device = nullptr;

	for (auto dev : m_devices) {
		if (!strcmp(dev->getDeviceId(), device_id)) {
			device = dev;
			break;
		}
	}
	return device;
}

void GenericXmlContext::addDevice(AbstractDevice* dev) { m_devices.push_back(dev); }

ssize_t GenericXmlContext::readData(char* buf, size_t len) { return socket_read(m_current_socket, buf, len); }

ssize_t GenericXmlContext::writeData(const char* buf, size_t len)
{
	m_current_socket->write(buf, len);
	return 0;
}

ssize_t GenericXmlContext::readLine(char* buf, size_t len)
{
	UNUSED(buf);
	UNUSED(len);
	return -ENOENT;
}

ssize_t GenericXmlContext::openInstance() { return -ENOENT; }

ssize_t GenericXmlContext::closeInstance() { return -ENOENT; }

ssize_t GenericXmlContext::readAttr(const char* device_id, const char* attr, char* buf, size_t len,
				    enum iio_attr_type type)
{
	return iio_emu::read_device_attr(m_doc, device_id, attr, buf, len, type);
}

ssize_t GenericXmlContext::writeAttr(const char* device_id, const char* attr, const char* buf, size_t len,
				     enum iio_attr_type type)
{
	return iio_emu::write_dev_attr(m_doc, device_id, attr, buf, len, type);
}

ssize_t GenericXmlContext::chReadAttr(const char* device_id, const char* channel, bool ch_out, const char* attr,
				      char* buf, size_t len)
{
	return iio_emu::read_channel_attr(m_doc, device_id, channel, ch_out, attr, buf, len);
}

ssize_t GenericXmlContext::chWriteAttr(const char* device_id, const char* channel, bool ch_out, const char* attr,
				       const char* buf, size_t len)
{
	return iio_emu::write_channel_attr(m_doc, device_id, channel, ch_out, attr, buf, len);
}

int32_t GenericXmlContext::openDev(const char* device, size_t sample_size, uint32_t mask, bool cyclic)
{
	AbstractDevice* abstractDevice = getDevice(device);
	if (abstractDevice) {
		abstractDevice->setDescriptor(m_current_socket->getDescriptor());
		return abstractDevice->open_dev(sample_size, mask, cyclic);
	}
	return -ENOENT;
}

int32_t GenericXmlContext::closeDev(const char* device)
{
	AbstractDevice* abstractDevice = getDevice(device);
	if (abstractDevice) {
		return abstractDevice->close_dev();
	}
	return -ENOENT;
}

ssize_t GenericXmlContext::transferDevToMem(const char* device, size_t bytes_count)
{
	AbstractDevice* abstractDevice = getDevice(device);
	if (abstractDevice) {
		auto* deviceIn = dynamic_cast<AbstractDeviceIn*>(abstractDevice);
		if (deviceIn) {
			return deviceIn->transfer_dev_to_mem(bytes_count);
		}
	}
	return -ENOENT;
}

ssize_t GenericXmlContext::readDev(const char* device, char* pbuf, size_t offset, size_t bytes_count)
{
	AbstractDevice* abstractDevice = getDevice(device);
	if (abstractDevice) {
		auto* deviceIn = dynamic_cast<AbstractDeviceIn*>(abstractDevice);
		if (deviceIn) {
			return deviceIn->read_dev(pbuf, offset, bytes_count);
		}
	}
	return -ENOENT;
}

ssize_t GenericXmlContext::transferMemToDev(const char* device, size_t bytes_count)
{
	AbstractDevice* abstractDevice = getDevice(device);
	if (abstractDevice) {
		auto* deviceOut = dynamic_cast<AbstractDeviceOut*>(abstractDevice);
		if (deviceOut) {
			return deviceOut->transfer_mem_to_dev(bytes_count);
		}
	}
	return -ENOENT;
}

ssize_t GenericXmlContext::writeDev(const char* device, const char* buf, size_t offset, size_t bytes_count)
{
	AbstractDevice* abstractDevice = getDevice(device);
	if (abstractDevice) {
		auto* deviceOut = dynamic_cast<AbstractDeviceOut*>(abstractDevice);
		if (deviceOut) {
			return deviceOut->write_dev(buf, offset, bytes_count);
		}
	}
	return -ENOENT;
}

int32_t GenericXmlContext::getMask(const char* device, uint32_t* mask)
{
	AbstractDevice* abstractDevice = getDevice(device);
	if (abstractDevice) {
		return abstractDevice->get_mask(mask);
	}
	return -ENOENT;
}

int32_t GenericXmlContext::setTimeout(uint32_t timeout)
{
	UNUSED(timeout);
	return -ENOENT;
}

int32_t GenericXmlContext::setBuffersCount(const char* device, uint32_t buffers_count)
{
	AbstractDevice* abstractDevice = getDevice(device);
	if (abstractDevice) {
		return abstractDevice->set_buffers_count(buffers_count);
	}
	return -ENOENT;
}

ssize_t GenericXmlContext::getXml(char** outxml)
{
	if (!outxml) {
		return -1;
	}

	*outxml = m_ctx_xml;
	return m_xml_size;
}

void GenericXmlContext::setCurrentSocket(AbstractSocket* socket) { m_current_socket = socket; }

void GenericXmlContext::socketDisconnected(int fd)
{
	AbstractDevice* abstractDevice = getDevice(fd);
	if (abstractDevice) {
		abstractDevice->cancel_buffer();
	}
}

void GenericXmlContext::assignBasicOps()
{
	m_iiodOps->read = iio_emu::read;
	m_iiodOps->write = iio_emu::write;

	m_iiodOps->read_attr = iio_emu::read_attr;
	m_iiodOps->write_attr = iio_emu::write_attr;
	m_iiodOps->ch_read_attr = iio_emu::ch_read_attr;
	m_iiodOps->ch_write_attr = iio_emu::ch_write_attr;

	m_iiodOps->transfer_dev_to_mem = transfer_dev_to_mem;
	m_iiodOps->read_data = iio_emu::read_data;
	m_iiodOps->transfer_mem_to_dev = iio_emu::transfer_mem_to_dev;
	m_iiodOps->write_data = iio_emu::write_data;

	m_iiodOps->open = iio_emu::open;
	m_iiodOps->close = iio_emu::close;

	m_iiodOps->get_mask = iio_emu::get_mask;
	m_iiodOps->set_buffers_count = iio_emu::set_buffers_count;
	m_iiodOps->get_xml = iio_emu::get_xml;
}

void GenericXmlContext::assignAllOps()
{
	m_iiodOps->read = iio_emu::read;
	m_iiodOps->write = iio_emu::write;
	m_iiodOps->read_line = iio_emu::read_line;

	m_iiodOps->open_instance = iio_emu::open_instance;
	m_iiodOps->close_instance = iio_emu::close_instance;

	m_iiodOps->read_attr = iio_emu::read_attr;
	m_iiodOps->write_attr = iio_emu::write_attr;
	m_iiodOps->ch_read_attr = iio_emu::ch_read_attr;
	m_iiodOps->ch_write_attr = iio_emu::ch_write_attr;

	m_iiodOps->transfer_dev_to_mem = transfer_dev_to_mem;
	m_iiodOps->read_data = iio_emu::read_data;
	m_iiodOps->transfer_mem_to_dev = iio_emu::transfer_mem_to_dev;
	m_iiodOps->write_data = iio_emu::write_data;

	m_iiodOps->open = iio_emu::open;
	m_iiodOps->close = iio_emu::close;

	m_iiodOps->get_mask = iio_emu::get_mask;
	m_iiodOps->set_timeout = iio_emu::set_timeout;
	m_iiodOps->set_buffers_count = iio_emu::set_buffers_count;
	m_iiodOps->get_xml = iio_emu::get_xml;
}

AbstractDevice* GenericXmlContext::getDevice(int fd)
{
	AbstractDevice* device = nullptr;

	for (auto dev : m_devices) {
		if (dev->getDescriptor() == fd) {
			device = dev;
			break;
		}
	}
	return device;
}

bool GenericXmlContext::isScanChannel(const char* device_id)
{
	xmlNode *root, *node_device, *node_channel, *node_attr;

	root = xmlDocGetRootElement(m_doc);
	if (root == nullptr) {
		return false;
	}

	node_device = getNode(root, "device", "id", device_id);
	if (node_device == nullptr) {
		return false;
	}

	for (node_channel = node_device->children; node_channel; node_channel = node_channel->next) {
		if (!strcmp(reinterpret_cast<const char*>(node_channel->name), "channel")) {
			node_attr = getNode(node_channel, "scan-element");
			if (node_attr) {
				return true;
			}
		}
	}

	return false;
}

bool GenericXmlContext::isOutputChannel(const char* device_id)
{
	if (!isScanChannel(device_id)) {
		return false;
	}

	xmlNode *root, *node_device, *node_attr;

	root = xmlDocGetRootElement(m_doc);
	if (root == nullptr) {
		return false;
	}

	node_device = getNode(root, "device", "id", device_id);
	if (node_device == nullptr) {
		return false;
	}

	node_attr = getNode(node_device, "channel", "type", "input");
	if (node_attr == nullptr) {
		return true;
	}

	return false;
}

bool GenericXmlContext::isInputChannel(const char* device_id)
{
	if (!isScanChannel(device_id)) {
		return false;
	}

	return !isOutputChannel(device_id);
}
