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

#ifndef IIO_EMU_UTILS_HPP
#define IIO_EMU_UTILS_HPP

#include <numeric>
#include <cstdint>
#include <string>
#include <vector>

namespace iio_emu {

#ifndef UNUSED
#define UNUSED(x)                                                                                                      \
	do {                                                                                                           \
		(void)(x);                                                                                             \
	} while (false)
#endif

constexpr uint8_t BYTE_SIZE = 8;

double safe_stod(const std::string& value);

template <typename T>
void analogical_decimation(const std::vector<T>& src, std::vector<T>& dest, unsigned int ratio)
{
	if (ratio < 2) {
		// return original vector
		dest = src;
		return;
	}

	size_t destSize = src.size() / ratio;
	if (dest.capacity() < destSize) {
		// resize dest
		dest.reserve(destSize);
	}

	for (auto it = src.begin(); it < src.end(); it += ratio) {
		dest.emplace_back(std::accumulate(it, it + ratio, 0.0) / ratio);
	}
}

template <typename T>
void analogical_interpolation(const std::vector<T>& src, std::vector<T>& dest, unsigned int ratio)
{
	if (ratio < 2) {
		// return original vector
		dest = src;
		return;
	}

	size_t destSize = ((src.size() - 1) * ratio + src.size());
	if (dest.capacity() < destSize) {
		// resize dest
		dest.reserve(destSize);
	}

	for (unsigned int i = 0; i < src.size() - 1; i++) {
		double val = (src.at(i + 1) - src.at(i)) / (ratio);
		dest.emplace_back(src.at(i));
		for (unsigned int j = 0; j < ratio; j++) {
			dest.emplace_back(dest.back() + val);
		}
	}
	dest.emplace_back(src.back());
}

template <typename T>
bool getBit(T number, T index)
{
	return (number >> index) & static_cast<T>(1u);
}

template <typename T>
static void setBit(T& number, T index)
{
	number |= static_cast<T>((1u << index));
}
template <typename T>
void digital_decimation(const std::vector<T>& src, std::vector<T>& dest, unsigned int ratio)
{
	if (ratio < 2) {
		// return original vector
		dest = src;
		return;
	}

	size_t destSize = src.size() / ratio;
	if (dest.capacity() < destSize) {
		// resize dest
		dest.reserve(destSize);
	}

	for (unsigned int i = 0; i < src.size(); i += ratio) {
		std::vector<unsigned int> bits_sum(sizeof(T) * BYTE_SIZE, 0);

		for (unsigned int j = 0; j < ratio; j++) {
			for (T k = 0; k < sizeof(T) * BYTE_SIZE; k++) {
				bits_sum.at(k) += static_cast<unsigned int>((getBit<T>(src.at(i + j), k) ? 1 : 0));
			}
		}
		T sample = 0;
		for (T k = 0; k < sizeof(T) * BYTE_SIZE; k++) {
			if (bits_sum.at(k) >= (ratio >> 1)) {
				setBit<T>(sample, k);
			}
		}
		dest.emplace_back(sample);
	}
}

template <typename T>
void digital_interpolation(const std::vector<T>& src, std::vector<T>& dest, unsigned int ratio)
{
	if (ratio < 2) {
		// return original vector
		dest = src;
		return;
	}

	size_t destSize = src.size() * ratio;
	if (dest.capacity() < destSize) {
		// resize dest
		dest.reserve(destSize);
	}

	for (auto sample : src) {
		for (unsigned int j = 0; j < ratio; j++) {
			dest.emplace_back(sample);
		}
	}
}

} // namespace iio_emu
#endif // IIO_EMU_UTILS_HPP
