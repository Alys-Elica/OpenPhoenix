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

#ifndef OFNX_GRAPHICS_RENDEREROPENGL_H
#define OFNX_GRAPHICS_RENDEREROPENGL_H

#include "ofnx/ofnx_globals.h"

#include <string>

namespace ofnx::graphics {

/**
 * @brief OpenGL rendering class.
 *
 * Renders both VR and static pictures.
 * SDL_Init(SDL_INIT_VIDEO) must be called before calling init
 */
class OFNX_EXPORT RendererOpenGL final {
public:
    RendererOpenGL();
    ~RendererOpenGL();

    using oglLoadFunc = void (*)(void);
    bool init(int width, int height, bool isNewVr, oglLoadFunc oglFct);
    void deinit();

    void updateVr(unsigned short* vr);
    void updateFrame(unsigned short* frame);
    void renderVr(int width, int height, float yaw, float pitch, float roll, float fov);
    void renderFrame();

private:
    class Impl;
    Impl* d_ptr;
};

} // namespace ofnx::graphics

#endif // OFNX_GRAPHICS_RENDEREROPENGL_H
