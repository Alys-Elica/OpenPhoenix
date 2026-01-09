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

#ifndef OFNX_OFNXMANAGER_H
#define OFNX_OFNXMANAGER_H

#include "ofnx/ofnx_globals.h"

#include <vector>

#include "ofnx/graphics/rendereropengl.h"

namespace ofnx {

/**
 * @brief Manages main resources (SDL init, renderer, event manager, etc.)
 */
class OFNX_EXPORT OfnxManager final {
public:
    struct Event {
        enum Type {
            Quit,
            MainMenu,
            MouseClickLeft,
            MouseClickRight,
            MouseMove,
            MouseWheel,
        };

        Type type;
        float x = 0;
        float y = 0;
        float xRel = 0;
        float yRel = 0;
    };

public:
    OfnxManager();
    ~OfnxManager();

    bool init(int width, int height, bool isNewVrVersion);
    void deinit();

    // TODO: might create a more flexible render manager (vulkan, D3D, etc.)
    ofnx::graphics::RendererOpenGL& renderer();

    std::vector<Event> getEvents();

private:
    class Impl;
    Impl* d_ptr;
};

} // namespace ofnx

#endif // OFNX_OFNXMANAGER_H
