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

#include "attr_ops_xml.hpp"

#include "xml_utils.hpp"

#include <libxml/tree.h>

ssize_t iio_emu::read_device_attr(struct _xmlDoc* doc, const char* device_id, const char* attr, char* buf, size_t len,
				  enum iio_attr_type type)
{
	xmlNode* node_attr;
	char* value;
	LIBXML_TEST_VERSION;

	if (!doc) {
		return -ENOENT;
	}

	node_attr = getDeviceAttr(doc, device_id, type, attr);
	if (node_attr == nullptr) {
		return -ENOENT;
	}

	value = reinterpret_cast<char*>(xmlGetProp(node_attr, reinterpret_cast<const xmlChar*>("value")));
	memcpy(buf, value, strnlen(value, len) + 1);
	xmlFree(value);

	return static_cast<ssize_t>(strnlen(buf, len) + 1);
}

ssize_t iio_emu::write_dev_attr(struct _xmlDoc* doc, const char* device_id, const char* attr, const char* buf,
				size_t len, enum iio_attr_type type)
{
	xmlNode* node_attr;
	LIBXML_TEST_VERSION;
	if (!doc) {
		return -ENOENT;
	}

	node_attr = getDeviceAttr(doc, device_id, type, attr);
	if (node_attr == nullptr) {
		return -ENOENT;
	}

	xmlSetProp(node_attr, reinterpret_cast<const xmlChar*>("value"), reinterpret_cast<const xmlChar*>(buf));

	return static_cast<ssize_t>(strnlen(buf, len) + 1);
}

ssize_t iio_emu::read_channel_attr(struct _xmlDoc* doc, const char* device_id, const char* channel, bool ch_out,
				   const char* attr, char* buf, size_t len)
{
	xmlNode* node_attr;
	char* value;
	LIBXML_TEST_VERSION;

	if (!doc) {
		return -ENOENT;
	}

	node_attr = getChannelAttr(doc, channel, device_id, attr, ch_out);
	if (node_attr == nullptr) {
		return -ENOENT;
	}
	value = reinterpret_cast<char*>(xmlGetProp(node_attr, reinterpret_cast<const xmlChar*>("value")));
	memcpy(buf, value, strnlen(value, len) + 1);
	xmlFree(value);

	return static_cast<ssize_t>(strnlen(buf, len) + 1);
}

ssize_t iio_emu::write_channel_attr(struct _xmlDoc* doc, const char* device_id, const char* channel, bool ch_out,
				    const char* attr, const char* buf, size_t len)
{
	xmlNode* node_attr;
	LIBXML_TEST_VERSION;

	if (!doc) {
		return -ENOENT;
	}

	node_attr = getChannelAttr(doc, channel, device_id, attr, ch_out);
	if (node_attr == nullptr) {
		return -ENOENT;
	}

	xmlSetProp(node_attr, reinterpret_cast<const xmlChar*>("value"), reinterpret_cast<const xmlChar*>(buf));

	return static_cast<ssize_t>(strnlen(buf, len) + 1);
}

ssize_t iio_emu::read_context_attr(struct _xmlDoc* doc, const char* attr, char* buf, size_t len)
{
	xmlNode* node_attr;
	char* value;

	LIBXML_TEST_VERSION;

	if (!doc) {
		return -ENOENT;
	}

	xmlNode* root = xmlDocGetRootElement(doc);

	node_attr = getNode(root, "context-attribute", "name", attr);
	value = reinterpret_cast<char*>(xmlGetProp(node_attr, reinterpret_cast<const xmlChar*>("value")));
	memcpy(buf, value, strnlen(value, len) + 1);
	xmlFree(value);

	return static_cast<ssize_t>(strnlen(buf, len) + 1);
}
