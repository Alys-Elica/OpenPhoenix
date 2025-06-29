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
