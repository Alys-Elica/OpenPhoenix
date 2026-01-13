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

#ifndef OFNX_GRAPHICS_DCT_H
#define OFNX_GRAPHICS_DCT_H

#include <cstdint>
#include <vector>

#include "ofnx/ofnx_globals.h"

namespace ofnx::graphics {

/**
 * @brief Decodes DCT encoded image (RGB565 or 32 bits ARGB)
 */
class OFNX_EXPORT Dct final {
public:
    Dct();
    ~Dct();

    Dct(const Dct& other) = delete;
    Dct& operator=(const Dct& other) = delete;

    /**
     * @brief Decodes DCT compressed data to RGB565 image data
     *
     * @param width Width
     * @param height Height
     * @param quality Quality
     * @param dataIn Input DCT compressed data
     * @param dataOut 16 bit RGB565 image
     */
    void unpackImageRgb16(
        int width, int height, int quality,
        std::vector<uint8_t>& dataIn,
        std::vector<uint16_t>& dataOut);

    /**
     * @brief Decodes DCT compressed data to ARGB image data
     *
     * @param width Width
     * @param height Height
     * @param quality Quality
     * @param dataIn Input DCT compressed data
     * @param dataOut 32 bit ARGB image
     */
    void unpackImageRgb32(
        int width, int height, int quality,
        std::vector<uint8_t>& dataIn,
        std::vector<uint32_t>& dataOut);

private:
    class Impl;
    Impl* d_ptr;
};

} // namespace ofnx::graphics

#endif // OFNX_GRAPHICS_DCT_H
