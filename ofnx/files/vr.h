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

#ifndef OFNX_FILES_VR_H
#define OFNX_FILES_VR_H

#include <cstdint>
#include <string>
#include <vector>

#include "ofnx/ofnx_globals.h"

namespace ofnx::files {

/**
 * @brief VR file reading class
 *
 * Loads pictures (640*480) and cubemap (256*6144) VR images.
 * Animation frames can also be applied directly to an output buffer.
 */
class OFNX_EXPORT Vr final {
public:
    enum class Type {
        VR_STATIC_PIC,
        VR_STATIC_VR,
        VR_UNKNOWN,
    };

public:
    Vr();
    ~Vr();

    Vr(const Vr& other) = delete;
    Vr& operator=(const Vr& other) = delete;

    /**
     * @brief Load VR file by name
     *
     * @param vrFileName VR file name
     */
    bool load(const std::string& vrFileName);

    /**
     * @brief Returns image width (0 if invalid)
     */
    int getWidth() const;

    /**
     * @brief Returns image height (0 if invalid)
     */
    int getHeight() const;

    /**
     * @brief Returns image type (Type::STATIC_PIC, Type::STATIC_VR, Type::STATIC_UNKNOWN)
     */
    Type getType() const;

    /**
     * @brief Unpacks image data into dataRgb565 using RGB565 pixel format
     *
     * @param dataRgb565 Output RGB565 buffer (resized automatically)
     */
    bool getDataRgb565(std::vector<uint16_t>& dataRgb565) const;

    /**
     * @brief Returns VR animation count
     */
    int getAnimationCount();

    /**
     * @brief Applies current animation frame to an RGB565 buffer
     *
     * Writes named animation current frame to the output and increments th the next frame
     * Rolls back to first animation frame if the last one is reached
     *
     * @param name Animation name
     * @param bufferOut Output buffer to apply frame into
     */
    bool applyAnimationFrameRgb565(const std::string& name, uint16_t* bufferOut);

private:
    class Impl;
    Impl* d_ptr;
};

} // namespace ofnx::files

#endif // OFNX_FILES_VR_H
