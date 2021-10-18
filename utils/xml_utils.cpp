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

#include "xml_utils.hpp"

#include <algorithm>
#include <libxml/tree.h>
#include <string>
#include <vector>

using namespace iio_emu;

struct _xmlNode* iio_emu::getNode(struct _xmlNode* root, const char* node_name, const char* attr_name,
				  const char* attr_value)
{
	xmlNode* node = nullptr;
	xmlAttr* attr = nullptr;

	for (node = root->children; node; node = node->next) {
		if (!strcmp(reinterpret_cast<const char*>(node->name), node_name)) {
			for (attr = node->properties; attr; attr = attr->next) {
				if (!strcmp(reinterpret_cast<const char*>(attr->name), attr_name)) {
					if (!strcmp(reinterpret_cast<const char*>(attr->children->content),
						    attr_value)) {
						return node;
					}
				}
			}
		}
	}
	return nullptr;
}

struct _xmlNode* iio_emu::getNode(struct _xmlNode* root, const char* node_name)
{
	xmlNode* node = nullptr;

	for (node = root->children; node; node = node->next) {
		if (!strcmp(reinterpret_cast<const char*>(node->name), node_name)) {
			return node;
		}
	}
	return nullptr;
}

static xmlNode* getNodeNAttrs(xmlNode* root, const char* node_name, const char** attr_name, const char** attr_value,
			      size_t len)
{
	xmlNode* node = nullptr;
	xmlAttr* attr[IIOD_BUFFER_SIZE];
	for (node = root->children; node; node = node->next) {
	search_node:
		if (!strcmp(reinterpret_cast<const char*>(node->name), node_name)) {
			for (size_t i = 0; i < len; i++) {
				for (attr[i] = node->properties; attr[i]; attr[i] = attr[i]->next) {
					if (!strcmp(reinterpret_cast<const char*>(attr[i]->name), attr_name[i])) {
						if (strcmp(reinterpret_cast<const char*>(attr[i]->children->content),
							   attr_value[i]) != 0) {
							node = node->next;
							goto search_node;
						}
					}
				}
			}
			return node;
		}
	}
	return nullptr;
}

xmlNode* iio_emu::getDeviceAttr(xmlDoc* doc, const char* dev, enum iio_attr_type type, const char* attr)
{
	xmlNode *root, *node_device, *node_attr;

	if (!doc) {
		return nullptr;
	}
	root = xmlDocGetRootElement(doc);
	if (root == nullptr) {
		return nullptr;
	}
	node_device = getNode(root, "device", "id", dev);
	if (node_device == nullptr) {
		return nullptr;
	}
	switch (type) {
	case IIO_ATTR_TYPE_DEVICE:
		node_attr = getNode(node_device, "attribute", "name", attr);
		break;
	case IIO_ATTR_TYPE_DEBUG:
		node_attr = getNode(node_device, "debug-attribute", "name", attr);
		break;
	case IIO_ATTR_TYPE_BUFFER:
		node_attr = getNode(node_device, "buffer-attribute", "name", attr);
		break;
	}
	return node_attr;
}

xmlNode* iio_emu::getChannelAttr(xmlDoc* doc, const char* chn, const char* dev, const char* attr, bool ch_out)
{

	xmlNode *root, *node_device, *node_channel, *node_attr;

	if (doc == nullptr) {
		return nullptr;
	}
	const char* output = ch_out ? "output" : "input";
	const char* attrs_name[] = {"id", "type"};
	const char* attrs_values[] = {chn, output};
	root = xmlDocGetRootElement(doc);
	if (root == nullptr) {
		return nullptr;
	}
	node_device = getNode(root, "device", "id", dev);
	if (node_device == nullptr) {
		return nullptr;
	}
	node_channel = getNodeNAttrs(node_device, "channel", attrs_name, attrs_values, 2);
	if (node_channel == nullptr) {
		return nullptr;
	}
	node_attr = getNode(node_channel, "attribute", "name", attr);
	return node_attr;
}

static void removeValueAttribute(struct _xmlNode* node)
{
	if (node == nullptr) {
		return;
	}
	removeValueAttribute(node->children);
	removeValueAttribute(node->next);

	if (strcmp(reinterpret_cast<const char*>(node->name), "context-attribute") != 0) {
		for (xmlAttr* attr = node->properties; attr; attr = attr->next) {
			if (!strcmp(reinterpret_cast<const char*>(attr->name), "value")) {
				xmlRemoveProp(attr);
				break;
			}
		}
	}
}

ssize_t iio_emu::getXml(struct _xmlDoc* doc, char** buf)
{
	if (!doc) {
		return -ENOENT;
	}

	xmlDoc* tmpDoc;
	xmlChar* xml;
	int size;

	LIBXML_TEST_VERSION

	xmlDocDumpMemory(doc, &xml, &size);

	tmpDoc = xmlReadMemory(reinterpret_cast<const char*>(xml), size, nullptr, nullptr, XML_PARSE_DTDVALID);
	xmlFree(xml);
	xmlNode* root = xmlDocGetRootElement(tmpDoc);
	removeValueAttribute(root);
	xmlDocDumpMemory(tmpDoc, &xml, &size);

	if (xml == nullptr) {
		return -ENOENT;
	}

	std::string str(reinterpret_cast<const char*>(xml));
	xmlFree(xml);

	str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
	std::string::iterator new_end =
		std::unique(str.begin(), str.end(), [&](char l, char r) { return (l == r) && (l == ' '); });
	str.erase(new_end, str.end());
	const char* content = str.c_str();

	*buf = new char[str.size() + 1];
	memcpy(*buf, content, str.size());
	(*buf)[str.size()] = '\0';

	xmlFreeDoc(tmpDoc);
	return static_cast<ssize_t>(str.size());
}
