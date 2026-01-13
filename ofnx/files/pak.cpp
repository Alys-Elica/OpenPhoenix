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

#include "pak.h"

#include <bitset>
#include <fstream>
#include <iostream>
#include <vector>

#include "ofnx/tools/datastream.h"
#include "ofnx/tools/log.h"

namespace ofnx::files {

/* PRIVATE */
struct PakFile {
    std::string fileName;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint32_t compressionLevel;

    std::vector<uint8_t> compressedData;
};

class Pak::Impl {
    friend class Pak;

public:
    static void uncompressPakData3(const std::vector<uint8_t>& dataIn, std::vector<uint8_t>& dataOut);

private:
    std::fstream filePak;
    std::vector<PakFile> listFile;
};

void Pak::Impl::uncompressPakData3(const std::vector<uint8_t>& dataIn, std::vector<uint8_t>& dataOut)
{
    size_t idxIn = 0;
    while (idxIn < dataIn.size()) {
        uint8_t byte = dataIn[idxIn++];

        if (std::bitset<8>(byte)[7]) {
            /*
             * If most significant bit is set to 1
             *      => copy of N1 bytes from already uncompressed data starting at N2 bytes from the end of
             *         current outputed data
             *              N1 = byte & 0x3f;
             *              N2 = index (1 byte if second most significant bit is 1, 2 bytes if 0)
             */

            int size = (byte & 0x3f) + 1;
            int index;

            if (std::bitset<8>(byte)[6]) {
                uint8_t tmpIndex = dataIn[idxIn++];
                index = dataOut.size() - (tmpIndex + 1);
            } else {
                uint16_t tmpIndex;

                // Stored in big endian
                tmpIndex = dataIn[idxIn++] << 8;
                tmpIndex |= dataIn[idxIn++];

                index = dataOut.size() - (tmpIndex + 1);
            }

            for (int i = 0; i < size; ++i) {
                dataOut.push_back(dataOut[index++]);
            }
        } else {
            for (int i = 0; i < byte + 1; ++i) {
                dataOut.push_back(dataIn[idxIn++]);
            }
        }
    }
}

/* PUBLIC */
Pak::Pak()
{
    d_ptr = new Impl;
}

Pak::~Pak()
{
    delete d_ptr;
}

bool Pak::open(const std::string& pakFileName)
{
    d_ptr->filePak.open(pakFileName, std::ios_base::in | std::ios_base::binary);
    if (!d_ptr->filePak.is_open()) {
        LOG_CRITICAL("Could not open file: {}", pakFileName);
        return false;
    }

    ofnx::tools::DataStream ds(&d_ptr->filePak);
    ds.setEndian(std::endian::little);

    uint8_t header[5] = { 0, 0, 0, 0, 0 };
    ds.read(4, header);

    uint32_t fileSize;
    ds >> fileSize;

    d_ptr->listFile.clear();
    while (!d_ptr->filePak.eof()) {
        PakFile subFile;

        uint8_t compFileName[16]; // Compressed file name
        ds.read(16, compFileName);
        subFile.fileName = std::string((char*)compFileName);

        ds >> subFile.compressionLevel;
        ds >> subFile.compressedSize;
        ds >> subFile.uncompressedSize;

        subFile.compressedData.resize(subFile.compressedSize, 0x00);
        ds.read(subFile.compressedSize, subFile.compressedData.data());

        d_ptr->listFile.push_back(subFile);

        // Check if we are at the end of the file
        if (d_ptr->filePak.tellg() == fileSize) {
            break;
        }
    }

    return true;
}

void Pak::close()
{
    d_ptr->filePak.close();
}

bool Pak::isOpen() const
{
    return d_ptr->filePak.is_open();
}

int Pak::fileCount() const
{
    return d_ptr->listFile.size();
}

std::string Pak::fileName(int index) const
{
    return d_ptr->listFile[index].fileName;
}

std::vector<uint8_t> Pak::fileData(int index) const
{
    if (index < 0 || index >= d_ptr->listFile.size()) {
        LOG_CRITICAL("Index out of range");
        return std::vector<uint8_t>();
    }

    if (!isOpen()) {
        LOG_CRITICAL("File not open");
        return std::vector<uint8_t>();
    }

    PakFile& subFile = d_ptr->listFile[index];

    std::vector<uint8_t> uncompressedData;
    uncompressedData.reserve(subFile.uncompressedSize);
    switch (subFile.compressionLevel) {
    case 3:
        Impl::uncompressPakData3(subFile.compressedData, uncompressedData);
        break;

    default:
        LOG_CRITICAL("Compression not yet known");
        break;
    }

    if (uncompressedData.size() != subFile.uncompressedSize) {
        LOG_CRITICAL("Uncompressed size does not match");
        LOG_CRITICAL("    Expected: {}", subFile.uncompressedSize);
        LOG_CRITICAL("    Actual: {}", uncompressedData.size());

        return std::vector<uint8_t>();
    }

    return uncompressedData;
}

} // namespace ofnx::files
