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

#include "ofnx/files/tst.h"

#include <fstream>
#include <vector>

#include "ofnx/tools/datastream.h"
#include "ofnx/tools/log.h"

namespace ofnx::files {

/* PRIVATE */
class Tst::Impl {
    friend class Tst;

public:
    struct Zone {
        uint32_t index;
        float x1;
        float x2;
        float y1;
        float y2;
    };

private:
    std::vector<Zone> m_listZone;
};

/* PUBLIC */
Tst::Tst()
{
    d_ptr = new Impl;
}

Tst::~Tst()
{
    delete d_ptr;
}

bool Tst::loadFile(const std::string& fileName)
{
    d_ptr->m_listZone.clear();

    std::fstream tstFile(fileName, std::ios::binary | std::ios::in);
    if (!tstFile.is_open()) {
        LOG_ERROR("Failed to open TST file");
        return false;
    }

    ofnx::tools::DataStream ds(&tstFile);
    ds.setEndian(std::endian::little);

    uint32_t zoneCount = 0;
    ds >> zoneCount;

    for (uint32_t i = 0; i < zoneCount; i++) {
        Impl::Zone zone;

        zone.index = i;

        ds >> zone.x1;
        ds >> zone.x2;
        ds >> zone.y1;
        ds >> zone.y2;

        d_ptr->m_listZone.push_back(zone);
    }

    return true;
}

int Tst::checkZoneStatic(float x, float y)
{
    for (const auto& zone : d_ptr->m_listZone) {
        float minX = std::min(zone.x1, zone.x2);
        float maxX = std::max(zone.x1, zone.x2);

        float minY = std::min(zone.y1, zone.y2);
        float maxY = std::max(zone.y1, zone.y2);

        if ((minX <= x) && (x <= maxX) && (minY <= y) && (y <= maxY)) {
            return zone.index;
        }
    }

    return -1;
}

int Tst::checkZoneVr(float yawDeg, float pitchDeg)
{
    float yaw = yawDeg * 0.0174532925f; // Convert to radians
    float pitch = pitchDeg * 0.0174532925f; // Convert to radians

    for (const auto& zone : d_ptr->m_listZone) {
        float minX = std::min(zone.x1, zone.x2);
        float maxX = std::max(zone.x1, zone.x2);
        if (3.141593f < maxX - minX) {
            float tmpX = maxX;
            maxX = minX + 6.283185f;
            minX = tmpX;
        }

        float minY = std::min(zone.y1, zone.y2);
        float maxY = std::max(zone.y1, zone.y2);
        if (3.141593f < maxY - minY) {
            float tmpY = maxY;
            maxY = minY + 6.283185f;
            minY = tmpY;
        }

        bool inX1 = (minX <= yaw) && (yaw <= maxX);
        bool inX2 = (minX <= yaw + 6.283185f) && (yaw + 6.283185f <= maxX);
        bool inY1 = (minY <= pitch) && (pitch <= maxY);
        bool inY2 = (minY <= pitch + 6.283185f) && (pitch + 6.283185f <= maxY);

        if ((inX1 || inX2) && (inY1 || inY2)) {
            return zone.index;
        }
    }

    return -1;
}

} // namespace ofnx::files
