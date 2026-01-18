/*
MIT License

Copyright (c) 2026 Alys_Elica

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

#include "datastream.h"

#include <cstring>
#include <iostream>

#include "ofnx/tools/log.h"

namespace ofnx::tools {

/* PRIVATE */
class DataStream::Impl {
    friend class DataStream;

public:
    void read(const size_t size, uint8_t* data);
    void read8(uint8_t& data);
    void read16(uint16_t& data);
    void read32(uint32_t& data);
    void read64(uint64_t& data);

    void write(const size_t size, const uint8_t* data);
    void write8(const uint8_t data);
    void write16(const uint16_t data);
    void write32(const uint32_t data);
    void write64(const uint64_t data);

private:
    std::fstream* m_file = nullptr;
    std::vector<uint8_t>* m_data = nullptr;
    std::endian m_endian;

    size_t m_pos = 0;
};

void DataStream::Impl::read(const size_t size, uint8_t* data)
{
    if (m_data) {
        if (m_pos + size > m_data->size()) {
            LOG_WARN("Out of range");
            return;
        }

        std::memcpy(data, m_data->data() + m_pos, size);
        m_pos += size;
    } else if (m_file) {
        if (m_file->eof()) {
            LOG_WARN("Out of range");
            return;
        }

        m_file->read(reinterpret_cast<char*>(data), size);
    } else {
        LOG_ERROR("No data source");
    }
}

void DataStream::Impl::read8(uint8_t& data)
{
    read(1, &data);
}

void DataStream::Impl::read16(uint16_t& data)
{
    uint8_t tmp[2];
    read8(tmp[0]);
    read8(tmp[1]);
    if (m_endian == std::endian::little) {
        data = tmp[0];
        data |= tmp[1] << 8;
    } else {
        data = tmp[0] << 8;
        data |= tmp[1];
    }
}

void DataStream::Impl::read32(uint32_t& data)
{
    uint16_t tmp[2];
    read16(tmp[0]);
    read16(tmp[1]);
    if (m_endian == std::endian::little) {
        data = tmp[0];
        data |= tmp[1] << 16;
    } else {
        data = tmp[0] << 16;
        data |= tmp[1];
    }
}

void DataStream::Impl::read64(uint64_t& data)
{
    uint32_t tmp[2];
    read32(tmp[0]);
    read32(tmp[1]);
    if (m_endian == std::endian::little) {
        data = tmp[0];
        data |= (uint64_t)tmp[1] << 32;
    } else {
        data = (uint64_t)tmp[0] << 32;
        data |= tmp[1];
    }
}

void DataStream::Impl::write(const size_t size, const uint8_t* data)
{
    if (m_data) {
        if (m_pos + size > m_data->size()) {
            m_data->resize(m_pos + size);
        }

        std::memcpy(m_data->data() + m_pos, data, size);
        m_pos += size;
    } else if (m_file) {
        m_file->write(reinterpret_cast<const char*>(data), size);
    } else {
        LOG_ERROR("No data source");
    }
}

void DataStream::Impl::write8(const uint8_t data)
{
    write(1, &data);
}

void DataStream::Impl::write16(const uint16_t data)
{
    uint8_t tmp[2];
    if (m_endian == std::endian::little) {
        tmp[0] = data & 0xFF;
        tmp[1] = (data >> 8) & 0xFF;
    } else {
        tmp[0] = (data >> 8) & 0xFF;
        tmp[1] = data & 0xFF;
    }
    write(2, tmp);
}

void DataStream::Impl::write32(const uint32_t data)
{
    uint16_t tmp[2];
    if (m_endian == std::endian::little) {
        tmp[0] = data & 0xFFFF;
        tmp[1] = (data >> 16) & 0xFFFF;
    } else {
        tmp[0] = (data >> 16) & 0xFFFF;
        tmp[1] = data & 0xFFFF;
    }
    write16(tmp[0]);
    write16(tmp[1]);
}

void DataStream::Impl::write64(const uint64_t data)
{
    uint32_t tmp[2];
    if (m_endian == std::endian::little) {
        tmp[0] = data & 0xFFFFFFFF;
        tmp[1] = (data >> 32) & 0xFFFFFFFF;
    } else {
        tmp[0] = (data >> 32) & 0xFFFFFFFF;
        tmp[1] = data & 0xFFFFFFFF;
    }
    write32(tmp[0]);
    write32(tmp[1]);
}

DataStream::DataStream()
{
    d_ptr = new Impl();
}

/* PUBLIC */
DataStream::DataStream(std::fstream* file)
    : DataStream()
{
    d_ptr->m_file = file;
}

DataStream::DataStream(std::vector<uint8_t>* data)
    : DataStream()
{
    d_ptr->m_data = data;
}

DataStream::~DataStream()
{
    delete d_ptr;
}

void DataStream::setEndian(const std::endian& endian)
{
    d_ptr->m_endian = endian;
}

// Read methods
void DataStream::read(const size_t size, uint8_t* data)
{
    d_ptr->read(size, data);
}

DataStream& DataStream::operator>>(uint8_t& data)
{
    d_ptr->read8(data);
    return *this;
}

DataStream& DataStream::operator>>(uint16_t& data)
{
    d_ptr->read16(data);
    return *this;
}

DataStream& DataStream::operator>>(uint32_t& data)
{
    d_ptr->read32(data);
    return *this;
}

DataStream& DataStream::operator>>(uint64_t& data)
{
    d_ptr->read64(data);
    return *this;
}

DataStream& DataStream::operator>>(int8_t& data)
{
    return *this >> reinterpret_cast<uint8_t&>(data);
}

DataStream& DataStream::operator>>(int16_t& data)
{
    return *this >> reinterpret_cast<uint16_t&>(data);
}

DataStream& DataStream::operator>>(int32_t& data)
{
    return *this >> reinterpret_cast<uint32_t&>(data);
}

DataStream& DataStream::operator>>(int64_t& data)
{
    return *this >> reinterpret_cast<uint64_t&>(data);
}

DataStream& DataStream::operator>>(float& data)
{
    uint32_t tmp;
    *this >> tmp;
    data = *reinterpret_cast<float*>(&tmp);
    return *this;
}

DataStream& DataStream::operator>>(double& data)
{
    uint64_t tmp;
    *this >> tmp;
    data = *reinterpret_cast<double*>(&tmp);
    return *this;
}

// Write methods
void DataStream::write(const size_t size, const uint8_t* data)
{
    d_ptr->write(size, data);
}

DataStream& DataStream::operator<<(const uint8_t data)
{
    d_ptr->write8(data);
    return *this;
}

DataStream& DataStream::operator<<(const uint16_t data)
{
    d_ptr->write16(data);
    return *this;
}

DataStream& DataStream::operator<<(const uint32_t data)
{
    d_ptr->write32(data);
    return *this;
}

DataStream& DataStream::operator<<(const uint64_t data)
{
    d_ptr->write64(data);
    return *this;
}

DataStream& DataStream::operator<<(const int8_t data)
{
    return *this << reinterpret_cast<const uint8_t&>(data);
}

DataStream& DataStream::operator<<(const int16_t data)
{
    return *this << reinterpret_cast<const uint16_t&>(data);
}

DataStream& DataStream::operator<<(const int32_t data)
{
    return *this << reinterpret_cast<const uint32_t&>(data);
}

DataStream& DataStream::operator<<(const int64_t data)
{
    return *this << reinterpret_cast<const uint64_t&>(data);
}

DataStream& DataStream::operator<<(const float data)
{
    return *this << *reinterpret_cast<const uint32_t*>(&data);
}

DataStream& DataStream::operator<<(const double data)
{
    return *this << *reinterpret_cast<const uint64_t*>(&data);
}

} // namespace ofnx::tools
