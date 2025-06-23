/*
MIT License

Copyright (c) 2025 Alys_Elica

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef OFNX_TOOLS_DATASTREAM_H
#define OFNX_TOOLS_DATASTREAM_H

#include <bit>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#include "ofnx/ofnx_globals.h"

namespace ofnx::tools {

/**
 * @brief Data stream class heavily inspired by Qt's QDataStream
 *
 * Allows to read basic variable types from a file/vector using C++ streams.
 * Endianness is also configurable (defaults to std::endian::big).
 */
class OFNX_EXPORT DataStream final {
public:
    DataStream(std::fstream* file);
    DataStream(std::vector<uint8_t>* data);
    ~DataStream();

    DataStream(const DataStream& other) = delete;
    DataStream& operator=(const DataStream& other) = delete;

    /**
     * @brief Set reading/writing endianness
     *
     * @param endian Endianness (little-endian or big-endian)
     */
    void setEndian(const std::endian& endian);

    // Read methods
    void read(const size_t size, uint8_t* data);

    DataStream& operator>>(uint8_t& data);
    DataStream& operator>>(uint16_t& data);
    DataStream& operator>>(uint32_t& data);
    DataStream& operator>>(uint64_t& data);

    DataStream& operator>>(int8_t& data);
    DataStream& operator>>(int16_t& data);
    DataStream& operator>>(int32_t& data);
    DataStream& operator>>(int64_t& data);

    DataStream& operator>>(float& data);
    DataStream& operator>>(double& data);

    // Write methods
    void write(const size_t size, const uint8_t* data);

    DataStream& operator<<(const uint8_t data);
    DataStream& operator<<(const uint16_t data);
    DataStream& operator<<(const uint32_t data);
    DataStream& operator<<(const uint64_t data);

    DataStream& operator<<(const int8_t data);
    DataStream& operator<<(const int16_t data);
    DataStream& operator<<(const int32_t data);
    DataStream& operator<<(const int64_t data);

    DataStream& operator<<(const float data);
    DataStream& operator<<(const double data);

private:
    DataStream();
    class Impl;
    Impl* d_ptr;
};

} // namespace ofnx::tools

#endif // OFNX_TOOLS_DATASTREAM_H
