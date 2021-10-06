/*
* libiio - Library for interfacing industrial I/O (IIO) devices
*
* Copyright (C) 2014 Analog Devices, Inc.
* Author: Paul Cercueil <paul.cercueil@analog.com>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* */

#include <stdio.h>
#include <iio.h>
#include <errno.h>
#include <string.h>
#include <libxml/tree.h>

#ifdef _MSC_BUILD
#define inline __inline
#define iio_snprintf sprintf_s
#define iio_sscanf sscanf_s
#else
#define iio_snprintf snprintf
#define iio_sscanf sscanf
#endif

#ifndef NAME_MAX
#define NAME_MAX 256
#endif
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

#define MAX_CHN_ID     NAME_MAX  /* encoded in the sysfs filename */
#define MAX_CHN_NAME   NAME_MAX  /* encoded in the sysfs filename */
#define MAX_DEV_ID     NAME_MAX  /* encoded in the sysfs filename */
#define MAX_DEV_NAME   NAME_MAX  /* encoded in the sysfs filename */
#define MAX_CTX_NAME   NAME_MAX  /* nominally "xml" */
#define MAX_CTX_DESC   NAME_MAX  /* nominally "linux ..." */
#define MAX_ATTR_NAME  NAME_MAX  /* encoded in the sysfs filename */
#define MAX_ATTR_VALUE PAGESIZE  /* Linux page size, could be anything */

enum iio_attr_type {
	IIO_ATTR_TYPE_DEVICE = 0,
	IIO_ATTR_TYPE_DEBUG,
	IIO_ATTR_TYPE_BUFFER,
};

size_t iio_strlcpy(char * __restrict dst, const char * __restrict src, size_t dsize)
{
	const char *osrc = src;
	size_t nleft = dsize;

	/* Copy as many bytes as will fit. */
	if (nleft != 0) {
		while (--nleft != 0) {
			if ((*dst++ = *src++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src. */
	if (nleft == 0) {
		if (dsize != 0)
			*dst = '\0';            /* NUL-terminate dst */

		while (*src++)
			;
	}

	return(src - osrc - 1); /* count does not include NUL */
}

char * encode_xml_ndup(const char * input)
{
	char * out;

	out = (char *)xmlEncodeEntitiesReentrant(NULL, (const xmlChar *)input);

	return out;
}

static const char xml_header[] = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
				 "<!DOCTYPE context ["
				 "<!ELEMENT context (device | context-attribute)*>"
				 "<!ELEMENT context-attribute EMPTY>"
				 "<!ELEMENT device (channel | attribute | debug-attribute | buffer-attribute)*>"
				 "<!ELEMENT channel (scan-element?, attribute*)>"
				 "<!ELEMENT attribute EMPTY>"
				 "<!ELEMENT scan-element EMPTY>"
				 "<!ELEMENT debug-attribute EMPTY>"
				 "<!ELEMENT buffer-attribute EMPTY>"
				 "<!ATTLIST context name CDATA #REQUIRED description CDATA #IMPLIED>"
				 "<!ATTLIST context-attribute name CDATA #REQUIRED value CDATA #REQUIRED>"
				 "<!ATTLIST device id CDATA #REQUIRED name CDATA #IMPLIED>"
				 "<!ATTLIST channel id CDATA #REQUIRED type (input|output) #REQUIRED name CDATA #IMPLIED>"
				 "<!ATTLIST scan-element index CDATA #REQUIRED format CDATA #REQUIRED scale CDATA #IMPLIED>"
				 "<!ATTLIST attribute name CDATA #REQUIRED filename CDATA #IMPLIED value CDATA #IMPLIED>"
				 "<!ATTLIST debug-attribute name CDATA #REQUIRED value CDATA #IMPLIED>"
				 "<!ATTLIST buffer-attribute name CDATA #REQUIRED value CDATA #IMPLIED>"
				 "]>";
#define BUF_SIZE 100

static char *get_attr_xml2(const struct iio_device *device, const char *attr, size_t *length, enum iio_attr_type type)
{
	size_t len;
	char *str;
	char *value;
	ssize_t ret;

	len = sizeof("<attribute name=\"\" />") - 1;
	len += strnlen(attr, MAX_ATTR_NAME);
	value = malloc(BUF_SIZE);
	switch(type){
	case IIO_ATTR_TYPE_DEVICE:
		ret = iio_device_attr_read(device, attr, value, BUF_SIZE);
		if (ret < 0) {
			strncpy(value, "ERROR\0", 6);
		}
		break;
	case IIO_ATTR_TYPE_DEBUG:
		len += (sizeof("debug-") - 1);
		ret = iio_device_debug_attr_read(device, attr, value, BUF_SIZE);
		if (ret < 0) {
			strncpy(value, "ERROR\0", 6);
		}
		break;
	case IIO_ATTR_TYPE_BUFFER:
		len += (sizeof("buffer-") - 1);
		ret = iio_device_buffer_attr_read(device, attr, value, BUF_SIZE);
		if (ret < 0) {
			strncpy(value, "ERROR\0", 6);
		}
		break;
	default:
		return NULL;
	}
	len += sizeof(" value=\"\"") - 1;
	len += strnlen(value, NAME_MAX);
	*length = len; /* just the chars */
	len++; /* room for terminating NULL */
	str = malloc(len);
	if (!str)
		return NULL;

	switch (type) {
	case IIO_ATTR_TYPE_DEVICE:
		iio_snprintf(str, len, "<attribute name=\"%s\" value=\"%s\" />", attr, value);
		break;
	case IIO_ATTR_TYPE_DEBUG:
		iio_snprintf(str, len, "<debug-attribute name=\"%s\" value=\"%s\" />", attr, value);
		break;
	case IIO_ATTR_TYPE_BUFFER:
		iio_snprintf(str, len, "<buffer-attribute name=\"%s\" value=\"%s\" />", attr, value);
		break;
	}

	return str;
}

char *iio_strdup(const char *str)
{
#if defined(_WIN32)
	return _strdup(str);
#elif defined(HAS_STRDUP)
	return strdup(str);
#else
	size_t len = strlen(str);
	char *buf = malloc(len + 1);

	if (buf)
		memcpy(buf, str, len + 1);
	return buf;
#endif
}


static char *get_attr_xml(const struct iio_channel *channel, const char *attr, size_t *length)
{
	char *str;
	char *value;
	size_t len;
	ssize_t ret;

	len = strnlen(attr, MAX_ATTR_NAME);
	len += sizeof("<attribute name=\"\" />") - 1;

	if (iio_channel_attr_get_filename(channel, attr)) {
		len += strnlen(iio_channel_attr_get_filename(channel, attr), NAME_MAX);
		len += sizeof(" filename=\"\"") - 1;
	}
	len += sizeof(" value=\"\"") - 1;
	value = malloc(BUF_SIZE);
	ret = iio_channel_attr_read(channel, attr, value, BUF_SIZE);
	if (ret < 0) {
		strncpy(value, "ERROR\0", 6);
	}
	len += strnlen(value, NAME_MAX);

	*length = len; /* just the chars */
	len++;         /* room for terminating NULL */
	str = malloc(len);
	if (!str)
		return NULL;

	if (iio_channel_attr_get_filename(channel, attr))
		iio_snprintf(str, len, "<attribute name=\"%s\" filename=\"%s\" value=\"%s\" />",
			     attr, iio_channel_attr_get_filename(channel, attr), value);
	else
		iio_snprintf(str, len, "<attribute name=\"%s\" value=\"%s\" />", attr, value);
	free(value);
	return str;
}

static char * get_scan_element(const struct iio_channel *chn, size_t *length)
{
	char buf[1024], repeat[12] = "", *str;
	const struct iio_data_format *format = iio_channel_get_data_format(chn);
	char processed = (format->is_fully_defined ? 'A' - 'a' : 0);

	if (format->repeat > 1)
		iio_snprintf(repeat, sizeof(repeat), "X%u", format->repeat);

	iio_snprintf(buf, sizeof(buf), "<scan-element index=\"%li\" "
				       "format=\"%ce:%c%u/%u%s&gt;&gt;%u\" />",
		     iio_channel_get_index(chn), format->is_be ? 'b' : 'l',
		     format->is_signed ? 's' + processed : 'u' + processed,
		     format->bits, format->length, repeat,
		     format->shift);

	if (format->with_scale) {
		char *ptr = strrchr(buf, '\0');
		iio_snprintf(ptr - 2, buf + sizeof(buf) - ptr + 2,
			     "scale=\"%f\" />", format->scale);
	}

	str = iio_strdup(buf);
	if (str)
		*length = strlen(str);
	return str;
}

char * iio_channel_get_xml(const struct iio_channel *chn, size_t *length)
{
	ssize_t len;
	char *ptr, *eptr, *str, **attrs, *scan_element = NULL;
	size_t *attrs_len, scan_element_len = 0;
	unsigned int i;

	len = sizeof("<channel id=\"\" type=\"\" ></channel>") - 1;
	len += strnlen(iio_channel_get_id(chn), MAX_CHN_ID);
	len += (iio_channel_is_output(chn) ? sizeof("output") : sizeof("input")) - 1;
	if (iio_channel_get_name(chn)) {
		len += sizeof(" name=\"\"") - 1;
		len += strnlen(iio_channel_get_name(chn), MAX_CHN_NAME);
	}

	if (iio_channel_is_scan_element(chn)) {
		scan_element = get_scan_element(chn, &scan_element_len);
		if (!scan_element)
			return NULL;
		else
			len += scan_element_len;
	}

	attrs_len = malloc(iio_channel_get_attrs_count(chn) * sizeof(*attrs_len));
	if (!attrs_len)
		goto err_free_scan_element;

	attrs = malloc(iio_channel_get_attrs_count(chn) * sizeof(*attrs));
	if (!attrs)
		goto err_free_attrs_len;

	for (i = 0; i < iio_channel_get_attrs_count(chn); i++) {
		char *xml = get_attr_xml(chn, iio_channel_get_attr(chn, i), &attrs_len[i]);
		if (!xml)
			goto err_free_attrs;
		attrs[i] = xml;
		len += attrs_len[i];
	}

	len++;  /* room for terminating NULL */
	str = malloc(len);
	if (!str)
		goto err_free_attrs;
	ptr = str;
	eptr = str + len;

	if (len > 0) {
		ptr += iio_snprintf(str, len, "<channel id=\"%s\"", iio_channel_get_id(chn));
		len = eptr - ptr;
	}

	if (iio_channel_get_name(chn) && len > 0) {
		ptr += iio_snprintf(ptr, len, " name=\"%s\"", iio_channel_get_name(chn));
		len = eptr - ptr;
	}

	if (len > 0) {
		ptr += iio_snprintf(ptr, len, " type=\"%s\" >", iio_channel_is_output(chn) ? "output" : "input");
		len = eptr - ptr;
	}

	if (iio_channel_is_scan_element(chn) && len > (ssize_t) scan_element_len) {
		memcpy(ptr, scan_element, scan_element_len); /* Flawfinder: ignore */
		ptr += scan_element_len;
		len -= scan_element_len;
	}

	for (i = 0; i < iio_channel_get_attrs_count(chn); i++) {
		if (len > (ssize_t) attrs_len[i]) {
			memcpy(ptr, attrs[i], attrs_len[i]); /* Flawfinder: ignore */
			ptr += attrs_len[i];
			len -= attrs_len[i];
		}
		free(attrs[i]);
	}

	free(scan_element);
	free(attrs);
	free(attrs_len);

	if (len > 0) {
		ptr += iio_strlcpy(ptr, "</channel>", len);
		len -= sizeof("</channel>") -1;
	}

	*length = ptr - str;

	/* NULL char should be left, and that is it */
	if (len != 1) {
		free(str);
		return NULL;
	}

	return str;

err_free_attrs:
	while (i--)
		free(attrs[i]);
	free(attrs);
err_free_attrs_len:
	free(attrs_len);
err_free_scan_element:
	if (iio_channel_is_scan_element(chn))
		free(scan_element);
	return NULL;
}

char * iio_device_get_xml(const struct iio_device *dev, size_t *length)
{
	ssize_t len;
	char *ptr, *eptr, *str, **attrs, **channels, **buffer_attrs, **debug_attrs;
	size_t *attrs_len, *channels_len, *buffer_attrs_len, *debug_attrs_len;
	unsigned int i, j, k;

	len = sizeof("<device id=\"\" ></device>") - 1;
	len += strnlen(iio_device_get_id(dev), MAX_DEV_ID);
	if (iio_device_get_name(dev)) {
		len += sizeof(" name=\"\"") - 1;
		len += strnlen(iio_device_get_name(dev), MAX_DEV_NAME);
	}

	attrs_len = malloc(iio_device_get_attrs_count(dev) * sizeof(*attrs_len));
	if (!attrs_len)
		return NULL;

	attrs = malloc(iio_device_get_attrs_count(dev) * sizeof(*attrs));
	if (!attrs)
		goto err_free_attrs_len;

	for (i = 0; i < iio_device_get_attrs_count(dev); i++) {
		char *xml = get_attr_xml2(dev, iio_device_get_attr(dev, i), &attrs_len[i], IIO_ATTR_TYPE_DEVICE);
		if (!xml)
			goto err_free_attrs;
		attrs[i] = xml;
		len += attrs_len[i];
	}

	channels_len = malloc(iio_device_get_channels_count(dev) * sizeof(*channels_len));
	if (!channels_len)
		goto err_free_attrs;

	channels = malloc(iio_device_get_channels_count(dev) * sizeof(*channels));
	if (!channels)
		goto err_free_channels_len;

	for (j = 0; j < iio_device_get_channels_count(dev); j++) {
		char *xml = iio_channel_get_xml(iio_device_get_channel(dev, j),
						&channels_len[j]);
		if (!xml)
			goto err_free_channels;
		channels[j] = xml;
		len += channels_len[j];
	}

	buffer_attrs_len = malloc(iio_device_get_buffer_attrs_count(dev) *
				  sizeof(*buffer_attrs_len));
	if (!buffer_attrs_len)
		goto err_free_channels;

	buffer_attrs = malloc(iio_device_get_buffer_attrs_count(dev) * sizeof(*buffer_attrs));
	if (!buffer_attrs)
		goto err_free_buffer_attrs_len;

	for (k = 0; k < iio_device_get_buffer_attrs_count(dev); k++) {
		char *xml = get_attr_xml2(dev, iio_device_get_buffer_attr(dev, k),
					  &buffer_attrs_len[k], IIO_ATTR_TYPE_BUFFER);
		if (!xml)
			goto err_free_buffer_attrs;
		buffer_attrs[k] = xml;
		len += buffer_attrs_len[k];
	}

	debug_attrs_len = malloc(iio_device_get_debug_attrs_count(dev) *
				 sizeof(*debug_attrs_len));
	if (!debug_attrs_len)
		goto err_free_buffer_attrs;

	debug_attrs = malloc(iio_device_get_debug_attrs_count(dev) * sizeof(*debug_attrs));
	if (!debug_attrs)
		goto err_free_debug_attrs_len;

	for (k = 0; k < iio_device_get_debug_attrs_count(dev); k++) {
		char *xml = get_attr_xml2(dev, iio_device_get_debug_attr(dev, k),
					  &debug_attrs_len[k], IIO_ATTR_TYPE_DEBUG);
		if (!xml)
			goto err_free_debug_attrs;
		debug_attrs[k] = xml;
		len += debug_attrs_len[k];
	}

	len++;  /* room for terminating NULL */
	str = malloc(len);
	if (!str)
		goto err_free_debug_attrs;
	eptr = str + len;
	ptr = str;

	if (len > 0) {
		ptr += iio_snprintf(str, len, "<device id=\"%s\"", iio_device_get_id(dev));
		len = eptr - ptr;
	}

	if (iio_device_get_name(dev) && len > 0) {
		ptr += iio_snprintf(ptr, len, " name=\"%s\"", iio_device_get_name(dev));
		len = eptr - ptr;
	}

	if (len > 0) {
		ptr += iio_strlcpy(ptr, " >", len);
		len -= 2;
	}

	for (i = 0; i < iio_device_get_channels_count(dev); i++) {
		if (len > (ssize_t) channels_len[i]) {
			memcpy(ptr, channels[i], channels_len[i]); /* Flawfinder: ignore */
			ptr += channels_len[i];
			len -= channels_len[i];
		}
		free(channels[i]);
	}

	free(channels);
	free(channels_len);

	for (i = 0; i < iio_device_get_attrs_count(dev); i++) {
		if (len > (ssize_t) attrs_len[i]) {
			memcpy(ptr, attrs[i], attrs_len[i]); /* Flawfinder: ignore */
			ptr += attrs_len[i];
			len -= attrs_len[i];
		}
		free(attrs[i]);
	}

	free(attrs);
	free(attrs_len);

	for (i = 0; i < iio_device_get_buffer_attrs_count(dev); i++) {
		if (len > (ssize_t) buffer_attrs_len[i]) {
			memcpy(ptr, buffer_attrs[i], buffer_attrs_len[i]); /* Flawfinder: ignore */
			ptr += buffer_attrs_len[i];
			len -= buffer_attrs_len[i];
		}
		free(buffer_attrs[i]);
	}

	free(buffer_attrs);
	free(buffer_attrs_len);

	for (i = 0; i < iio_device_get_debug_attrs_count(dev); i++) {
		if (len > (ssize_t) debug_attrs_len[i]) {
			memcpy(ptr, debug_attrs[i], debug_attrs_len[i]); /* Flawfinder: ignore */
			ptr += debug_attrs_len[i];
			len -= debug_attrs_len[i];
		}
		free(debug_attrs[i]);
	}

	free(debug_attrs);
	free(debug_attrs_len);

	if (len > 0) {
		ptr += iio_strlcpy(ptr, "</device>", len);
		len -= sizeof("</device>") - 1;
	}

	*length = ptr - str;

	if (len != 1) {
		free(str);
		return NULL;
	}

	return str;

err_free_debug_attrs:
	while (k--)
		free(debug_attrs[k]);
	free(debug_attrs);
err_free_debug_attrs_len:
	free(debug_attrs_len);
err_free_buffer_attrs:
	while (k--)
		free(buffer_attrs[k]);
	free(buffer_attrs);
err_free_buffer_attrs_len:
	free(buffer_attrs_len);
err_free_channels:
	while (j--)
		free(channels[j]);
	free(channels);
err_free_channels_len:
	free(channels_len);
err_free_attrs:
	while (i--)
		free(attrs[i]);
	free(attrs);
err_free_attrs_len:
	free(attrs_len);
	return NULL;
}

char * iio_context_create_xml(const struct iio_context *ctx)
{
	ssize_t len;
	size_t *devices_len = NULL;
	char *str, *ptr, *eptr, **devices = NULL;
	char ** ctx_attrs, **ctx_values;
	char *ctx_attr, *ctx_value;
	unsigned int i;

	len = sizeof(xml_header) - 1;
	len += strnlen(iio_context_get_name(ctx), MAX_CTX_NAME);
	len += sizeof("<context name=\"\" ></context>") - 1;

	if (iio_context_get_description(ctx)) {
		len += strnlen(iio_context_get_description(ctx), MAX_CTX_DESC);
		len += sizeof(" description=\"\"") - 1;
	}

	ctx_attrs = calloc(iio_context_get_attrs_count(ctx), sizeof(char*));
	if (!ctx_attrs) {
		errno = ENOMEM;
		return NULL;
	}

	ctx_values = calloc(iio_context_get_attrs_count(ctx), sizeof(char*));
	if (!ctx_values) {
		errno = ENOMEM;
		goto err_free_ctx_attrs;
	}

	for (i = 0; i < iio_context_get_attrs_count(ctx); i++) {
		iio_context_get_attr(ctx, i, &ctx_attr, &ctx_value);
		ctx_attrs[i] = encode_xml_ndup(ctx_attr);
		ctx_values[i] = encode_xml_ndup(ctx_value);
		if (!ctx_attrs[i] || !ctx_values[i])
			goto err_free_ctx_attrs_values;

		len += strnlen(ctx_attrs[i], MAX_ATTR_NAME);
		len += strnlen(ctx_values[i], MAX_ATTR_VALUE);
		len += sizeof("<context-attribute name=\"\" value=\"\" />") - 1;
	}

	if (iio_context_get_devices_count(ctx)) {
		devices_len = malloc(iio_context_get_devices_count(ctx) * sizeof(*devices_len));
		if (!devices_len) {
			errno = ENOMEM;
			goto err_free_ctx_attrs_values;
		}

		devices = calloc(iio_context_get_devices_count(ctx), sizeof(*devices));
		if (!devices)
			goto err_free_devices_len;

		for (i = 0; i < iio_context_get_devices_count(ctx); i++) {
			char *xml = iio_device_get_xml(iio_context_get_device(ctx, i),
						       &devices_len[i]);
			if (!xml)
				goto err_free_devices;
			devices[i] = xml;
			len += devices_len[i];
		}
	}

	len++; /* room for terminating NULL */
	str = malloc(len);
	if (!str) {
		errno = ENOMEM;
		goto err_free_devices;
	}
	eptr = str + len;
	ptr = str;

	if (len > 0) {
		if (iio_context_get_description(ctx)) {
			ptr += iio_snprintf(str, len, "%s<context name=\"%s\" "
						      "description=\"%s\" >",
					    xml_header, iio_context_get_name(ctx), iio_context_get_description(ctx));
		} else {
			ptr += iio_snprintf(str, len, "%s<context name=\"%s\" >",
					    xml_header, iio_context_get_name(ctx));
		}
		len = eptr - ptr;
	}

	for (i = 0; i < iio_context_get_attrs_count(ctx) && len > 0; i++) {
		ptr += iio_snprintf(ptr, len, "<context-attribute name=\"%s\" value=\"%s\" />",
				    ctx_attrs[i], ctx_values[i]);
		free(ctx_attrs[i]);
		free(ctx_values[i]);
		len = eptr - ptr;
	}

	free(ctx_attrs);
	free(ctx_values);

	for (i = 0; i < iio_context_get_devices_count(ctx); i++) {
		if (len > (ssize_t) devices_len[i]) {
			memcpy(ptr, devices[i], devices_len[i]); /* Flawfinder: ignore */
			ptr += devices_len[i];
			len -= devices_len[i];
		}
		free(devices[i]);
	}

	free(devices);
	free(devices_len);

	if (len > 0) {
		ptr += iio_strlcpy(ptr, "</context>", len);
		len -= sizeof("</context>") - 1;
	}

	if (len != 1) {
		free(str);
		return NULL;
	}

	return str;

err_free_devices:
	for (i = 0; i < iio_context_get_devices_count(ctx); i++)
		free(devices[i]);
	free(devices);
err_free_devices_len:
	free(devices_len);
err_free_ctx_attrs_values:
	for (i = 0; i < iio_context_get_attrs_count(ctx); i++) {
		if (ctx_attrs[i])
			free(ctx_attrs[i]);
		if (ctx_values[i])
			free(ctx_values[i]);
	}

	free(ctx_values);
err_free_ctx_attrs:
	free(ctx_attrs);
	return NULL;
}

int main(int argc, const char *argv[])
{
	if(argc <= 1) {
		perror("Please provide the URI\n");
		exit(1);
	}

	struct iio_context *context = iio_create_context_from_uri(argv[1]);
	if (!context) {
		perror("Cannot open IIO context");
		exit(1);
	}

	const char *xml = iio_context_create_xml(context);
	printf(xml);
	return 0;
}
