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

#ifndef OFNX_GLOBALS_H
#define OFNX_GLOBALS_H

#ifdef _MSC_VER // Microsoft Visual Studio

#ifdef OFNX_EXPORTS
#define OFNX_EXPORT __declspec(dllexport)
#else
#define OFNX_EXPORT __declspec(dllimport)
#endif

#elif defined(__GNUC__) || defined(__clang__) // GCC or Clang

#ifdef OFNX_EXPORTS
#define OFNX_EXPORT __attribute__((visibility("default")))
#else
#define OFNX_EXPORT
#endif

#else

#error "Compiler not supported!"

#endif

#endif // OFNX_GLOBALS_H
