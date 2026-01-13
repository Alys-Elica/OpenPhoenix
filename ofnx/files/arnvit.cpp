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

#include "arnvit.h"

#include <fstream>
#include <iostream>
#include <map>

#include "ofnx/tools/datastream.h"
#include "ofnx/tools/log.h"

namespace ofnx::files {

/* PRIVATE */
class ArnVit::Impl {
    friend class ArnVit;

private:
    std::fstream fileVit;
    std::fstream fileArn;

    std::vector<ArnVit::ArnVitFile> fileList;
    std::map<std::string, int> fileNameMap;
};

/* PUBLIC */
ArnVit::ArnVit()
{
    d_ptr = new Impl();
}

ArnVit::~ArnVit()
{
    delete d_ptr;
}

bool ArnVit::open(const std::string& vitFileName, const std::string& arnFileName)
{
    d_ptr->fileVit.open(vitFileName, std::ios::binary | std::ios::in);
    if (!d_ptr->fileVit.is_open()) {
        LOG_CRITICAL("Unable to open VIT file ", vitFileName);
        return false;
    }

    d_ptr->fileArn.open(arnFileName, std::ios::binary | std::ios::in);
    if (!d_ptr->fileArn.is_open()) {
        LOG_CRITICAL("Unable to open ARN file ", arnFileName);
        return false;
    }

    d_ptr->fileList.clear();

    // Parse VIT header file
    ofnx::tools::DataStream ds(&d_ptr->fileVit);
    ds.setEndian(std::endian::little);

    uint32_t fileCount;
    ds >> fileCount;

    uint32_t unkn;
    ds >> unkn;

    uint32_t offset = 0;
    for (uint32_t i = 0; i < fileCount; i++) {
        ArnVitFile file;

        char fileName[32];
        ds.read(32, (uint8_t*)fileName);
        file.fileName = std::string(fileName);

        ds >> file.unkn1;
        ds >> file.unkn2;
        ds >> file.width;
        ds >> file.height;
        ds >> file.unkn3;
        ds >> file.fileSize;
        ds >> file.unkn4;

        file.offset = offset;

        offset += file.fileSize;

        d_ptr->fileList.push_back(file);

        d_ptr->fileNameMap[file.fileName] = i;
    }

    return true;
}

void ArnVit::close()
{
    d_ptr->fileVit.close();
    d_ptr->fileArn.close();
}

bool ArnVit::isOpen() const
{
    return d_ptr->fileVit.is_open() && d_ptr->fileArn.is_open();
}

int ArnVit::fileCount() const
{
    return d_ptr->fileList.size();
}

ArnVit::ArnVitFile ArnVit::getFile(const int index) const
{
    ArnVitFile file = d_ptr->fileList[index];

    d_ptr->fileArn.seekg(file.offset);
    file.data.resize(file.fileSize);
    d_ptr->fileArn.read((char*)file.data.data(), file.fileSize);

    return file;
}

ArnVit::ArnVitFile ArnVit::getFile(const std::string& name) const
{
    auto it = d_ptr->fileNameMap.find(name);
    if (it == d_ptr->fileNameMap.end()) {
        return {};
    }

    return getFile(it->second);
}

bool ArnVit::writeToBmp(const int index, const std::string& outputDirectory) const
{
    ArnVitFile file = getFile(index);
    if (file.data.empty()) {
        LOG_CRITICAL("Unable to read file data");
        return false;
    }

    std::string bmpFile = outputDirectory + file.fileName;
    std::fstream fileBmp(bmpFile, std::ios::binary | std::ios::out);
    if (!fileBmp.is_open()) {
        LOG_CRITICAL("Unable to open BMP file: {}", bmpFile);
        return false;
    }

    ofnx::tools::DataStream ds(&fileBmp);
    ds.setEndian(std::endian::little);

    // BMP header
    ds << uint16_t(0x4D42); // BM
    ds << uint32_t(0); // File size (will be updated later)
    ds << uint16_t(0); // Reserved
    ds << uint16_t(0); // Reserved
    ds << uint32_t(54); // Offset to image data

    // DIB header
    ds << uint32_t(40); // DIB header size
    ds << uint32_t(file.width); // Width
    ds << uint32_t(-file.height); // Height
    ds << uint16_t(1); // Planes
    ds << uint16_t(16); // Bits per pixel
    ds << uint32_t(0); // Compression
    ds << uint32_t(0); // Image size (ignored for uncompressed images)
    ds << uint32_t(0); // X pixels per meter
    ds << uint32_t(0); // Y pixels per meter
    ds << uint32_t(0); // Colors in color table
    ds << uint32_t(0); // Important color count

    size_t fileSize = 54;

    // Image data
    bool addPadding = (file.width * 2) % 4 != 0;
    for (uint32_t y = 0; y < file.height; y++) {
        // fileBmp.write(data.data() + y * file.width * 2, file.width * 2);
        for (uint32_t x = 0; x < file.width; x++) {
            int idx = (y * file.width + x) * 2;
            uint16_t pixel = (file.data[idx + 1] << 8) | file.data[idx];

            // RGB565 to RGB555
            pixel = ((pixel & 0b1111'1000'0000'0000) >> 1 | (pixel & 0b0000'0111'1100'0000) >> 1 | (pixel & 0b0000'0000'0001'1111));

            ds << pixel;

            fileSize += 2;
        }

        if (addPadding) {
            ds << uint8_t(0x00);
            ds << uint8_t(0xFF);

            fileSize += 2;
        }
    }

    // Write file size
    fileBmp.seekp(2);
    ds << uint32_t(fileSize);

    fileBmp.close();

    return true;
}

} // namespace ofnx::files
