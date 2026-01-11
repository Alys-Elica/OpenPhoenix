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

#include "vr.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <map>

#include "ofnx/graphics/dct.h"
#include "ofnx/tools/datastream.h"

namespace ofnx::files {

#define VR_FILE_HEADER 0x12FA84AB
#define VR_TYPE_PIC 0xA0B1C400
#define VR_TYPE_VR 0xA0B1C200
#define VR_TYPE_ANIMATION 0xA0B1C201
#define VR_TYPE_ANIMATION_FRAME 0xA0B1C211

#define VR2_FILE_HEADER 0x44414548 // HEAD
#define VR2_TYPE_PIC 0x43505453 // STPC
#define VR2_TYPE_VR 0x50575453 // STWP
#define VR2_TYPE_ANIMATION 0x50574E41 // ANWP
#define VR2_TYPE_ANIMATION_FRAME 0x4D415246 // FRAM

/* PRIVATE */
class Vr::Impl {
    friend class Vr;

private:
    struct AnimFrame {
        std::vector<uint32_t> blockOffsetList;
        std::vector<uint8_t> dctData;
        uint32_t dctQuality;
    };

    struct Anim {
        std::vector<AnimFrame> frameList;
        uint32_t currentFrame = 0;
    };

private:
    Vr::Type m_vrType;

    std::vector<uint8_t> m_dctData;
    uint32_t m_dctQuality;
    std::map<std::string, Anim> m_animationList;
};

/* PUBLIC */
Vr::Vr()
{
    d_ptr = new Impl;
}

Vr::~Vr()
{
    delete d_ptr;
}

bool Vr::load(const std::string& vrFileName)
{
    clear();

    std::fstream fileIn(vrFileName, std::ios::binary | std::ios::in);
    if (!fileIn.is_open()) {
        std::cerr << "[vr] Failed to open file" << std::endl;
        return false;
    }

    fileIn.seekg(0, std::ios::end);
    size_t fileSize = fileIn.tellg();
    fileIn.seekg(0, std::ios::beg);

    ofnx::tools::DataStream ds(&fileIn);
    ds.setEndian(std::endian::little);

    uint32_t chunkType = 0;
    uint32_t chunkSize = 0;
    ds >> chunkType;
    ds >> chunkSize;

    if (chunkType != VR_FILE_HEADER && chunkType != VR2_FILE_HEADER) {
        std::cerr << "[vr] Wrong file header" << std::endl;
        return false;
    }

    if (chunkSize != fileSize) {
        std::cerr << "[vr] Wrong file size" << std::endl;
        return false;
    }

    while (chunkType != 0) {
        ds >> chunkType;
        ds >> chunkSize;

        // TODO: better end of file check
        if (fileIn.eof()) {
            break;
        }

        if (chunkType == VR_TYPE_PIC || chunkType == VR_TYPE_VR
            || chunkType == VR2_TYPE_PIC || chunkType == VR2_TYPE_VR) {
            if (d_ptr->m_vrType != Type::VR_UNKNOWN) {
                std::cerr << "[vr] Multiple image data in file" << std::endl;
                return false;
            }

            uint32_t dcDataSize;

            ds >> d_ptr->m_dctQuality;
            ds >> dcDataSize;

            d_ptr->m_dctData.resize(dcDataSize);
            ds.read(dcDataSize, d_ptr->m_dctData.data());

            ofnx::graphics::Dct dct;
            switch (chunkType) {
            case VR_TYPE_PIC:
                d_ptr->m_vrType = Type::VR_STATIC_PIC;
                break;
            case VR_TYPE_VR:
                d_ptr->m_vrType = Type::VR_STATIC_VR;
                break;
            case VR2_TYPE_PIC:
                d_ptr->m_vrType = Type::VR2_STATIC_PIC;
                break;
            case VR2_TYPE_VR:
                d_ptr->m_vrType = Type::VR2_STATIC_VR;
                break;
            }
        } else if (chunkType == VR_TYPE_ANIMATION || chunkType == VR2_TYPE_ANIMATION) {
            char strName[0x20];
            ds.read(0x20, (uint8_t*)strName);
            std::string animName(strName);

            uint32_t frameCount;
            ds >> frameCount;

            d_ptr->m_animationList.insert({ animName, {} });
            for (uint32_t idxFrame = 0; idxFrame < frameCount; ++idxFrame) {
                uint32_t subChunkType;
                uint32_t subChunkSize;
                ds >> subChunkType;
                ds >> subChunkSize;

                if (subChunkType != VR_TYPE_ANIMATION_FRAME && subChunkType != VR2_TYPE_ANIMATION_FRAME) {
                    std::cerr << "[vr] Unknown animation sub-chunk type "
                              << std::hex << subChunkType << std::dec
                              << " -> ignoring" << std::endl;
                    fileIn.seekg(subChunkSize - 8, std::ios::cur);
                }

                if (subChunkSize - 8 == 0) {
                    // Empty frame -> skipping
                    d_ptr->m_animationList[animName].frameList.push_back({});
                    continue;
                }

                Impl::AnimFrame animFrame;

                uint32_t blockCount;
                ds >> blockCount;

                for (uint32_t idxBlock = 0; idxBlock < blockCount; ++idxBlock) {
                    uint32_t blockOffset;
                    ds >> blockOffset;

                    animFrame.blockOffsetList.push_back(blockOffset);
                }

                ds >> animFrame.dctQuality;
                uint32_t frameDctDataSize;
                ds >> frameDctDataSize;

                animFrame.dctData.resize(frameDctDataSize);
                ds.read(frameDctDataSize, animFrame.dctData.data());

                d_ptr->m_animationList[animName].frameList.push_back(animFrame);
            }
        } else {
            std::cerr << "[vr] Unknown chunk type at offset 0x"
                      << std::hex << (fileIn.tellg() - 8) << std::dec << ": "
                      << std::hex << chunkType << std::dec
                      << " -> ignoring" << std::endl;
            fileIn.seekg(chunkSize - 8, std::ios::cur);
        }
    }

    return true;
}

void Vr::clear()
{
    d_ptr->m_dctData.clear();
    d_ptr->m_animationList.clear();
    d_ptr->m_vrType = Type::VR_UNKNOWN;
}

int Vr::getWidth() const
{
    switch (getType()) {
    case Type::VR_STATIC_VR:
    case Type::VR2_STATIC_VR:
        return 256;
    case Type::VR_STATIC_PIC:
    case Type::VR2_STATIC_PIC:
        return 640;
    default:
        return 0;
    }
}

int Vr::getHeight() const
{
    switch (getType()) {
    case Type::VR_STATIC_VR:
    case Type::VR2_STATIC_VR:
        return 6144;
    case Type::VR_STATIC_PIC:
    case Type::VR2_STATIC_PIC:
        return 480;
    default:
        return 0;
    }
}

Vr::Type Vr::getType() const
{
    return d_ptr->m_vrType;
}

bool Vr::getDataRgb565(std::vector<uint16_t>& dataRgb565) const
{
    ofnx::graphics::Dct dct;
    switch (getType()) {
    case Type::VR_STATIC_VR:
    case Type::VR2_STATIC_VR:
        dct.unpackImageRgb16(256, 6144, d_ptr->m_dctQuality, d_ptr->m_dctData, dataRgb565);
        break;
    case Type::VR_STATIC_PIC:
    case Type::VR2_STATIC_PIC:
        dct.unpackImageRgb16(640, 480, d_ptr->m_dctQuality, d_ptr->m_dctData, dataRgb565);
        break;
    default:
        return false;
    }

    return true;
}

bool Vr::applyAnimationFrameRgb565(const std::string& name, uint16_t* bufferOut)
{
    auto anim = d_ptr->m_animationList.find(name);
    if (anim == d_ptr->m_animationList.end()) {
        // std::cerr << "[vr] Animation not found" << std::endl;
        return false;
    }

    Impl::AnimFrame& frame = anim->second.frameList.at(anim->second.currentFrame++);

    if (frame.blockOffsetList.empty()) {
        return true;
    }

    std::vector<uint16_t> dataRgb565;
    ofnx::graphics::Dct dct;
    dct.unpackImageRgb16(8, 8 * frame.blockOffsetList.size(), frame.dctQuality, frame.dctData, dataRgb565);

    int width = getWidth();
    for (int idxBlock = 0; idxBlock < frame.blockOffsetList.size(); ++idxBlock) {
        for (int idxPix = 0; idxPix < 64; ++idxPix) {
            int inIdx = (idxPix % 8) + (idxPix / 8) * 8 + (64 * idxBlock);
            int outIdx = (idxPix % 8) + (idxPix / 8) * width + frame.blockOffsetList[idxBlock];
            bufferOut[outIdx] = dataRgb565[inIdx];
        }
    }

    if (anim->second.currentFrame >= anim->second.frameList.size()) {
        anim->second.currentFrame = 0;
    }

    return true;
}

} // namespace ofnx::files
