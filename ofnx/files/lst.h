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

#ifndef OFNX_FILES_LST_H
#define OFNX_FILES_LST_H

#include "ofnx/ofnx_globals.h"

#include <string>
#include <variant>
#include <vector>

namespace ofnx::files {

class OFNX_EXPORT Lst final {
public:
    struct Instruction {
        std::string name;
        std::vector<std::string> params;
        std::vector<Instruction> subInstructions; // For ifand/ifor/plugin/subroutine
    };
    using InstructionBlock = std::vector<Instruction>;

public:
    Lst();
    ~Lst();

    bool parseLst(const std::string& fileName);
    bool saveLst(const std::string& fileName);

    InstructionBlock& getInitBlock(const std::string& warpName);
    InstructionBlock& getTestBlock(const std::string& warpName, const int& testId);

private:
    class Impl;
    Impl* d_ptr;
};

} // namespace ofnx::files

#endif // OFNX_FILES_LST_H
