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

#include "dct.h"

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "ofnx/tools/datastream.h"

namespace ofnx::graphics {

/* PRIVATE IMPLEMENTATION */
constexpr int QUANT_LUMA[64] = {
    0x10, 0x0B, 0x0A, 0x10, 0x18, 0x28, 0x33, 0x3D,
    0x0C, 0x0C, 0x0E, 0x13, 0x1A, 0x3A, 0x3C, 0x37,
    0x0E, 0x0D, 0x10, 0x18, 0x28, 0x39, 0x45, 0x38,
    0x0E, 0x11, 0x16, 0x1D, 0x33, 0x57, 0x50, 0x3E,
    0x12, 0x16, 0x25, 0x38, 0x44, 0x6D, 0x67, 0x4D,
    0x18, 0x23, 0x37, 0x40, 0x51, 0x68, 0x71, 0x5C,
    0x31, 0x40, 0x4E, 0x57, 0x67, 0x79, 0x78, 0x65,
    0x48, 0x5C, 0x5F, 0x62, 0x70, 0x64, 0x67, 0x63
};

constexpr int QUANT_CHROMA[64] = {
    0x11, 0x12, 0x18, 0x2F, 0x63, 0x63, 0x63, 0x63,
    0x12, 0x15, 0x1A, 0x42, 0x63, 0x63, 0x63, 0x63,
    0x18, 0x1A, 0x38, 0x63, 0x63, 0x63, 0x63, 0x63,
    0x2F, 0x42, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63
};

constexpr int DCT_QUANT_MULTIPLIERS[64] = {
    0x4000, 0x58C5, 0x539F, 0x4B42, 0x4000, 0x3249, 0x22A3, 0x11A8,
    0x58C5, 0x7B21, 0x73FC, 0x6862, 0x58C5, 0x45BF, 0x300B, 0x187E,
    0x539F, 0x73FC, 0x6D41, 0x6254, 0x539F, 0x41B3, 0x2D41, 0x1712,
    0x4B42, 0x6862, 0x6254, 0x587E, 0x4B42, 0x3B21, 0x28BA, 0x14C3,
    0x4000, 0x58C5, 0x539F, 0x4B42, 0x4000, 0x3249, 0x22A3, 0x11A8,
    0x3249, 0x45BF, 0x41B3, 0x3B21, 0x3249, 0x2782, 0x1B37, 0x0DE0,
    0x22A3, 0x300B, 0x2D41, 0x28BA, 0x22A3, 0x1B37, 0x12BF, 0x098E,
    0x11A8, 0x187E, 0x1712, 0x14C3, 0x11A8, 0x0DE0, 0x098E, 0x04DF
};

constexpr int ZIGZAG[64] = {
    0, 1, 8, 16, 9, 2, 3, 10,
    17, 24, 32, 25, 18, 11, 4, 5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13, 6, 7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

class Dct::Impl {
    friend class Dct;

public:
    bool unpack(
        int width, int height, int quality,
        std::vector<uint8_t>& data);

    void setQuality(int quality);
    void prepareQuant(const int quant[64], int block[64]);
    bool unpackBlock(int block[64]);
    void idct(int* block);
    void saveBlockRgba24(int index);

    char readAcData(int length);
    char readDcData(int length);

    void huffmanDecode(
        const std::vector<uint8_t>& dataIn,
        const int outputSize,
        std::vector<uint8_t>& dataOut);

private:
    int m_quantLuma[64];
    int m_quantChroma[64];
    int m_quality;
    int m_blockY[64];
    int m_blockCb[64];
    int m_blockCr[64];
    std::vector<uint8_t> m_dataImageBufferRgb24;
    std::vector<uint8_t> m_dataDcBuffer;
    std::vector<uint8_t> m_dataAcCodeBuffer;
    std::vector<uint8_t> m_dataAcBuffer;
    uint32_t m_dataDcIndex;
    uint32_t m_dataDcMask;
    uint32_t m_dataAcCodeIndex;
    uint32_t m_dataAcIndex;
    uint32_t m_dataAcMask;
};

bool Dct::Impl::unpack(int width, int height, int quality, std::vector<uint8_t>& data)
{
    // Checks
    if (width <= 0 || width > 10'000) {
        return false;
    }
    if (height <= 0 || height > 10'000) {
        return false;
    }
    if (data.empty()) {
        return false;
    }

    // Unpacking
    ofnx::tools::DataStream ds(&data);
    ds.setEndian(std::endian::little);

    // AC code data
    uint32_t dataAcCodeCompSize;
    uint32_t dataAcCodeSize;
    ds >> dataAcCodeCompSize;
    ds >> dataAcCodeSize;

    std::vector<uint8_t> dataAcCodeCompBuffer(dataAcCodeCompSize);
    ds.read(dataAcCodeCompSize, dataAcCodeCompBuffer.data());

    huffmanDecode(dataAcCodeCompBuffer, dataAcCodeSize, m_dataAcCodeBuffer);

    // AC data
    uint32_t dataAcSize;
    ds >> dataAcSize;

    m_dataAcBuffer.resize(dataAcSize);
    ds.read(dataAcSize, m_dataAcBuffer.data());

    // DC data
    uint32_t dataDcSize;
    ds >> dataDcSize;

    m_dataDcBuffer.resize(dataDcSize);
    ds.read(dataDcSize, m_dataDcBuffer.data());

    // Init unpack
    m_dataDcIndex = 0;
    m_dataDcMask = 0x80;
    m_dataAcCodeIndex = 0;
    m_dataAcIndex = 0;
    m_dataAcMask = 0x80;

    // Init quant tables
    setQuality(quality);
    prepareQuant(QUANT_LUMA, m_quantLuma);
    prepareQuant(QUANT_CHROMA, m_quantChroma);

    // Unpack
    m_dataImageBufferRgb24.resize(width * height * 3);
    int blockIdx = 0;
    for (int h = 0; h < (height / 8); ++h) {
        for (int w = 0; w < (width / 8); ++w) {
            int blockY[64];
            int blockCb[64];
            int blockCr[64];

            // Unpack blocks
            unpackBlock(blockY);
            unpackBlock(blockCb);
            unpackBlock(blockCr);

            // Set natural order
            for (int i = 0; i < 64; ++i) {
                m_blockY[ZIGZAG[i]] = blockY[i];
                m_blockCb[ZIGZAG[i]] = blockCb[i];
                m_blockCr[ZIGZAG[i]] = blockCr[i];
            }

            // Dequant
            for (int i = 0; i < 64; ++i) {
                m_blockY[i] *= m_quantLuma[i];
                m_blockCb[i] *= m_quantChroma[i];
                m_blockCr[i] *= m_quantChroma[i];
            }

            // iDCT
            idct(m_blockY);
            idct(m_blockCb);
            idct(m_blockCr);

            // YCbCr to RGB565
            // Stores block pixels sequentially
            saveBlockRgba24(blockIdx++);
        }
    }

    return true;
}

void Dct::Impl::setQuality(int quality)
{
    if (quality < 0) {
        m_quality = 5000;
        return;
    }

    if (quality < 101) {
        if (quality < 50) {
            m_quality = (int)(5000 / (long long)quality);
            return;
        }
    } else {
        quality = 100;
    }

    m_quality = (100 - quality) * 2;
}

void Dct::Impl::prepareQuant(const int quant[64], int block[64])
{
    for (int i = 0; i < 64; ++i) {
        block[i] = (quant[i] * m_quality + 50) / 100;
        if (block[i] < 8) {
            block[i] = 8;
        } else if (255 < block[i]) {
            block[i] = 255;
        }

        // Normalize
        block[i] = (DCT_QUANT_MULTIPLIERS[i] * block[i]) >> 13;
    }
}

bool Dct::Impl::unpackBlock(int block[64])
{
    for (int i = 0; i < 64; ++i) {
        block[i] = 0;
    }

    block[0] = readDcData(8);
    for (int idx = 1; idx < 64;) {
        unsigned char acCode = m_dataAcCodeBuffer[m_dataAcCodeIndex++];
        if (acCode == 0) {
            break;
        } else if (acCode == 0xf0) {
            idx += 16;
        } else {
            int offset = acCode >> 4;
            int size = acCode & 0xf;
            if (size == 0) {
                // 0 coeff
                return false;
            }

            int level = readAcData(size);
            if ((level & (1 << (size - 1))) == 0) {
                level += (1 - (1 << size));
            }

            idx += offset;
            if (idx >= 64) {
                return true;
            }

            block[idx++] = level;
        }
    }

    return true;
}

void Dct::Impl::idct(int* block)
{
    int* tmpBlock = block + 8;
    for (int row = 8; row > 0; --row) {
        int a0 = tmpBlock[-8] + tmpBlock[0x18];
        int a1 = tmpBlock[-8] - tmpBlock[0x18];
        int a2 = tmpBlock[8] + tmpBlock[0x28];
        int a3 = ((tmpBlock[8] - tmpBlock[0x28]) * 0x16a0a >> 16) - a2;
        int a4 = a0 + a2;
        int a5 = a0 - a2;
        int a6 = a3 + a1;
        int a7 = a1 - a3;
        int a8 = tmpBlock[0x10] + tmpBlock[0x20];
        int a9 = tmpBlock[0x20] - tmpBlock[0x10];
        int a10 = tmpBlock[0x30] + *tmpBlock;
        int a11 = *tmpBlock - tmpBlock[0x30];
        int a12 = a10 + a8;
        int a13 = (a11 + a9) * 0x1d907 >> 16;
        int a14 = ((a9 * -0x29cf6 >> 16) - a12) + a13;
        tmpBlock[-8] = a12 + a4;
        int a15 = ((a10 - a8) * 0x16a0a >> 16) - a14;
        tmpBlock[0x30] = a4 - a12;
        int a16 = ((a11 * 0x11518 >> 16) - a13) + a15;
        tmpBlock[0] = a14 + a6;
        tmpBlock[0x28] = a6 - a14;
        tmpBlock[0x20] = a7 - a15;
        tmpBlock[0x18] = a16 + a5;
        tmpBlock[8] = a15 + a7;
        tmpBlock[0x10] = a5 - a16;
        tmpBlock++;
    }

    tmpBlock = block + 1;
    for (int row = 8; row > 0; --row) {
        int a0 = tmpBlock[-1] + tmpBlock[3];
        int a1 = tmpBlock[-1] - tmpBlock[3];
        int a2 = tmpBlock[1] + tmpBlock[5];
        int a4 = ((tmpBlock[1] - tmpBlock[5]) * 0x16a0a >> 16) - a2;
        int a3 = a2 + a0;
        int a5 = a0 - a2;
        int a6 = a4 + a1;
        int a7 = a1 - a4;
        int a8 = tmpBlock[2] + tmpBlock[4];
        int a9 = tmpBlock[4] - tmpBlock[2];
        int a10 = tmpBlock[6] + *tmpBlock;
        int a11 = *tmpBlock - tmpBlock[6];
        int a12 = a10 + a8;
        int a13 = (a11 + a9) * 0x1d907 >> 16;
        int a14 = ((a9 * -0x29cf6 >> 16) - a12) + a13;
        tmpBlock[-1] = a12 + a3;
        int a15 = ((a10 - a8) * 0x16a0a >> 16) - a14;
        tmpBlock[6] = a3 - a12;
        int a16 = ((a11 * 0x11518 >> 16) - a13) + a15;
        tmpBlock[0] = a14 + a6;
        tmpBlock[5] = a6 - a14;
        tmpBlock[4] = a7 - a15;
        tmpBlock[3] = a16 + a5;
        tmpBlock[1] = a15 + a7;
        tmpBlock[2] = a5 - a16;
        tmpBlock += 8;
    }

    for (int i = 0; i < 64; i++) {
        int value = ((int)(block[i] + (block[i] >> 31 & 0xF)) >> 4);
        block[i] = std::clamp(value, -128, 128);
    }
}

void Dct::Impl::saveBlockRgba24(int index)
{
    for (int idxPix = 0; idxPix < 64; ++idxPix) {
        int tmpY = m_blockY[idxPix] + 128;
        int tmpCb = (int)((unsigned long long)((long long)m_blockCb[idxPix] * 0x55555555ll) >> 32) - m_blockCb[idxPix];

        int outIdx = (index * 64 * 3) + idxPix * 3;
        m_dataImageBufferRgb24[outIdx] = std::clamp(tmpY + m_blockCb[idxPix] * 2, 0, 255); // R
        m_dataImageBufferRgb24[outIdx + 1] = std::clamp((((tmpCb >> 1) - (tmpCb >> 31)) - (m_blockCr[idxPix] * 8) / 10) + tmpY, 0, 255); // G
        m_dataImageBufferRgb24[outIdx + 2] = std::clamp((m_blockCr[idxPix] << 4) / 10 + tmpY, 0, 255); // B
    }
}

char Dct::Impl::readAcData(int length)
{
    unsigned int mask = 1;
    char data = 0;
    for (int i = 0; i < length; ++i) {
        if (m_dataAcIndex >= m_dataAcBuffer.size()) {
            return 0;
        }
        unsigned char bit = m_dataAcBuffer[m_dataAcIndex] & m_dataAcMask;
        m_dataAcMask >>= 1;
        if (m_dataAcMask == 0) {
            m_dataAcMask = 0x80;
            m_dataAcIndex++;
        }

        if (bit != 0) {
            data |= mask;
        }
        mask <<= 1;
    }
    return data;
}

char Dct::Impl::readDcData(int length)
{
    unsigned int mask = 1;
    char data = 0;
    for (int i = 0; i < length; ++i) {
        unsigned char bit = m_dataDcBuffer[m_dataDcIndex] & m_dataDcMask;
        m_dataDcMask >>= 1;
        if (m_dataDcMask == 0) {
            m_dataDcMask = 0x80;
            m_dataDcIndex++;
        }

        if (bit != 0) {
            data |= mask;
        }
        mask <<= 1;
    }
    return data;
}

void Dct::Impl::huffmanDecode(
    const std::vector<uint8_t>& dataIn,
    const int outputSize,
    std::vector<uint8_t>& dataOut)
{
    dataOut.resize(outputSize);

    int buffer[513 * 3 + 1] = { 0 };
    size_t dataOffset = 0;

    // Read frequencies
    while (dataOffset < 256) {
        int blockStart = dataIn[dataOffset++];
        if (dataOffset > 1 && blockStart == 0) { // Reached end null-termination
            break;
        }

        int blockEnd = dataIn[dataOffset++];
        int blockLen = blockEnd - blockStart;
        for (int i = 0; i <= blockLen; i++) {
            buffer[(blockStart + i) * 3] = dataIn[dataOffset++];
        }
    }

    buffer[513 * 3] = INT_MAX;

    // Build Huffman tree
    size_t len = 256;
    buffer[len * 3] = 1;
    while (len < 513) {
        size_t left = 513;
        size_t right = 513;

        for (size_t index = 0; index <= len; index++) {
            if (buffer[index * 3] == 0) {
                continue;
            } else if (buffer[index * 3] < buffer[left * 3]) {
                right = left;
                left = index;
            } else if (buffer[index * 3] < buffer[right * 3]) {
                right = index;
            }
        }

        if (right == 513) {
            break;
        }

        ++len;

        buffer[len * 3] = buffer[left * 3] + buffer[right * 3];
        buffer[len * 3 + 1] = (int)left;
        buffer[len * 3 + 2] = (int)right;

        buffer[left * 3] = buffer[right * 3] = 0;
    }

    // Decode compressed bitstream
    unsigned char bitmask = 128;
    for (int i = 0; i < outputSize; i++) {
        int index = (int)len;
        while (index > 256) {
            if (bitmask & dataIn[dataOffset]) {
                index = buffer[index * 3 + 2];
            } else {
                index = buffer[index * 3 + 1];
            }

            bitmask >>= 1;

            if (bitmask == 0) {
                dataOffset++;
                bitmask = 128;
            }
        }

        if (index == 256) {
            break;
        }

        dataOut[i] = index;
    }
}

/* PUBLIC */
Dct::Dct()
{
    d_ptr = new Impl;
}

Dct::~Dct()
{
    delete d_ptr;
}

void Dct::unpackImageRgb16(
    int width, int height, int quality,
    std::vector<uint8_t>& dataIn,
    std::vector<uint16_t>& dataOut)
{
    if (!d_ptr->unpack(width, height, quality, dataIn)) {
        return;
    }

    // Reconstruct image
    dataOut.resize(width * height);
    int idxBlock = 0;
    int idxBlockPix = 0;
    int blockPerLine = (width / 8);
    for (int i = 0; i < d_ptr->m_dataImageBufferRgb24.size(); i += 3) {
        int w = idxBlock % blockPerLine;
        int h = idxBlock / blockPerLine;

        int outIdx = (w * 8) + (idxBlockPix % 8) + ((h * 8) + idxBlockPix / 8) * width;
        dataOut[outIdx]
            = ((d_ptr->m_dataImageBufferRgb24[i + 2] & 0xF8) << 8)
            | ((d_ptr->m_dataImageBufferRgb24[i + 1] & 0xFC) << 3)
            | (d_ptr->m_dataImageBufferRgb24[i + 0] >> 3);

        ++idxBlockPix;
        if (idxBlockPix >= 64) {
            ++idxBlock;
            idxBlockPix = 0;
        }
    }
}

void Dct::unpackImageRgb32(
    int width, int height, int quality,
    std::vector<uint8_t>& dataIn,
    std::vector<uint32_t>& dataOut)
{
    if (!d_ptr->unpack(width, height, quality, dataIn)) {
        return;
    }

    // Reconstruct image
    dataOut.resize(width * height);
    int idxBlock = 0;
    int idxBlockPix = 0;
    int blockPerLine = (width / 8);
    for (int i = 0; i < d_ptr->m_dataImageBufferRgb24.size(); i += 3) {
        int w = idxBlock % blockPerLine;
        int h = idxBlock / blockPerLine;

        int outIdx = (w * 8) + (idxBlockPix % 8) + ((h * 8) + idxBlockPix / 8) * width;
        dataOut[outIdx]
            = ((d_ptr->m_dataImageBufferRgb24[i + 0] & 0xFF) << 16)
            | ((d_ptr->m_dataImageBufferRgb24[i + 1] & 0xFF) << 8)
            | ((d_ptr->m_dataImageBufferRgb24[i + 2] & 0xFF) << 0)
            | (0xFF << 24);

        ++idxBlockPix;
        if (idxBlockPix >= 64) {
            ++idxBlock;
            idxBlockPix = 0;
        }
    }
}

} // namespace ofnx::graphics
