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

#ifndef OFNX_FILES_ARNVIT_H
#define OFNX_FILES_ARNVIT_H

#include "ofnx/ofnx_globals.h"

#include <cstdint>
#include <string>
#include <vector>

namespace ofnx::files {

class OFNX_EXPORT ArnVit final {
public:
    struct ArnVitFile {
        std::string fileName;
        uint32_t width;
        uint32_t height;
        uint32_t fileSize;
        uint32_t offset;

        uint32_t unkn1;
        uint32_t unkn2;
        uint32_t unkn3;
        uint32_t unkn4;

        std::vector<uint8_t> data;
    };

public:
    ArnVit();
    ~ArnVit();

    ArnVit(const ArnVit& other) = delete;
    ArnVit& operator=(const ArnVit& other) = delete;

    bool open(const std::string& vitFileName, const std::string& arnFileName);
    void close();
    bool isOpen() const;

    int fileCount() const;
    ArnVitFile getFile(const int index) const;
    ArnVitFile getFile(const std::string& name) const;
    bool writeToBmp(const int index, const std::string& outputDirectory) const;

private:
    class Impl;
    Impl* d_ptr;
};

} // namespace ofnx::files

#endif // OFNX_FILES_ARNVIT_H
