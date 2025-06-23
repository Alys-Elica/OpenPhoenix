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

#include "ofnxmanager.h"

#include <iostream>

#include <SDL3/SDL.h>

namespace ofnx {

/* PRIVATE */
class OfnxManager::Impl {
    friend class OfnxManager;

private:
    ofnx::graphics::RendererOpenGL m_renderer;
};

/* PUBLIC */
OfnxManager::OfnxManager()
{
    d_ptr = new Impl;
}

OfnxManager::~OfnxManager()
{
    delete d_ptr;
}

bool OfnxManager::init(int width, int height)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed - " << SDL_GetError() << std::endl;
        return 1;
    }

    if (!d_ptr->m_renderer.init(width, height)) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return false;
    }

    return true;
}

void OfnxManager::deinit()
{
    d_ptr->m_renderer.deinit();

    SDL_Quit();
}

ofnx::graphics::RendererOpenGL& OfnxManager::renderer()
{
    return d_ptr->m_renderer;
}

std::vector<OfnxManager::Event> OfnxManager::getEvents()
{
    std::vector<Event> eventList;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            eventList.push_back({ Event::Quit });
        } else if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_ESCAPE) {
                eventList.push_back({ Event::Quit });
            } else if (event.key.key == SDLK_RETURN) {
                eventList.push_back({ Event::MainMenu });
            }
        } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
            eventList.push_back(Event { Event::MouseMove, event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel });
        } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                eventList.push_back({ Event::MouseClickLeft, event.button.x, event.button.y });
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
                eventList.push_back({ Event::MouseClickRight, event.button.x, event.button.y });
            }
        } else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
            eventList.push_back({ Event::MouseWheel, event.wheel.x, event.wheel.y });
        }
    }

    return eventList;
}

} // namespace ofnx
