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

#include "lst.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>

namespace ofnx::files {

/* Helper functions */
inline void trim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); })
                .base(),
        s.end());
}

std::vector<std::string> split(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        trim(token);

        tokens.push_back(token);
    }
    return tokens;
}

/* PRIVATE */
struct Warp {
    std::string name;
    Lst::InstructionBlock initBlock;
    std::map<uint32_t, Lst::InstructionBlock> testBlockList;
};

class Lst::Impl {
    friend class Lst;

public:
    bool nextLine(std::string& line);

    bool parseVariable(const std::string& line, std::string& varaiableName);
    bool parseWarp(const std::string& line, std::string& warpName);
    bool parseTest(const std::string& line, int& test);
    bool parsePlugin(const std::string& line, Lst::Instruction& instruction);
    bool parseSubroutine(const std::string& line, Lst::Instruction& instruction);

    bool parseInstruction(const std::string& line, Lst::Instruction& instruction);
    bool parsePluginInstruction(const std::string& line, Lst::Instruction& instruction);

    bool addVariable(const std::string& name);
    bool addInstruction(const std::string& warpName, const int& testId, const Instruction& instruction);

    void optimize();

private:
    // Parsing data
    std::fstream m_file;
    int m_currentLine = 0;

    // Final data
    std::set<std::string> m_listVariables;
    std::map<std::string, Warp> m_listWarps;
    std::map<std::string, Instruction> m_listSubroutines;

    std::string m_initWarp;
};

bool Lst::Impl::nextLine(std::string& line)
{
    if (!std::getline(m_file, line)) {
        return false;
    }

    ++m_currentLine;

    // Lower case the line
    std::transform(line.begin(), line.end(), line.begin(), ::tolower);

    // Remove comments
    line = line.substr(0, line.find_first_of(';'));

    // Trim the line
    trim(line);

    // Skip empty lines
    if (line.empty()) {
        return nextLine(line);
    }

    return true;
}

bool Lst::Impl::parseVariable(const std::string& line, std::string& varaiableName)
{
    if (line.find("[bool]") != std::string::npos) {
        varaiableName = line.substr(line.find('=') + 1);
        trim(varaiableName);

        return true;
    }

    return false;
}

bool Lst::Impl::parseWarp(const std::string& line, std::string& warpName)
{
    if (line.find("[warp]") != std::string::npos) {
        warpName = line.substr(line.find('=') + 1, line.find(',') - line.find('=') - 1);
        trim(warpName);

        // Remove '.vr' if it exists
        if (warpName.find(".vr") != std::string::npos) {
            warpName = warpName.substr(0, warpName.find(".vr"));
        }

        return true;
    }

    return false;
}

bool Lst::Impl::parseTest(const std::string& line, int& test)
{
    if (line.find("[test]") != std::string::npos) {
        std::string testStr = line.substr(line.find('=') + 1);
        trim(testStr);

        // TODO: manage optional test number (e.g. [test]=6,1)
        test = std::stoi(testStr);

        if (test < -1) {
            std::cerr << m_currentLine << " Error: invalid test number: " << test << std::endl;
            return false;
        }

        return true;
    }

    return false;
}

bool Lst::Impl::parsePlugin(const std::string& line, Lst::Instruction& instruction)
{
    if (line == "plugin") {
        instruction.name = "plugin";

        // Parse instructions
        std::string line;
        while (nextLine(line)) {
            if (line == "endplugin") {
                // End of plugin
                break;
            }

            Lst::Instruction subInstruction;
            if (parsePluginInstruction(line, subInstruction)) {
                instruction.subInstructions.push_back(subInstruction);
                continue;
            }

            // Error
            std::cerr << m_currentLine << " Error: unknown line in plugin: " << line << std::endl;
            return false;
        }

        return true;
    }

    return false;
}

bool Lst::Impl::parseSubroutine(const std::string& line, Lst::Instruction& instruction)
{
    if (line.find("label") != std::string::npos) {
        std::string labelName = line.substr(line.find(' ') + 1);

        instruction.name = labelName;

        // Parse instructions
        std::string line;
        while (nextLine(line)) {
            if (line == "return") {
                // End of subroutine
                break;
            }

            Lst::Instruction subInstructionPlugin;
            if (parsePlugin(line, subInstructionPlugin)) {
                instruction.subInstructions.push_back(subInstructionPlugin);
                continue;
            }

            Lst::Instruction subInstruction;
            if (parseInstruction(line, subInstruction)) {
                instruction.subInstructions.push_back(subInstruction);
                continue;
            }

            // Error
            std::cerr << m_currentLine << " Error: unknown line in subroutine: " << line << std::endl;
            return false;
        }

        return true;
    }

    return false;
}

bool Lst::Impl::parseInstruction(const std::string& line, Lst::Instruction& instruction)
{
    std::string instructionName = line.substr(0, line.find_first_of(" ="));
    trim(instructionName);

    std::string params = line.substr(line.find_first_of(" =") + 1);
    trim(params);

    enum class InstrParamType {
        DOUBLE,
        STRING
    };
    struct InstrTmp {
        std::string name;
        std::vector<InstrParamType> params;
        char separator = ',';
    };
    std::vector<InstrTmp> instrTmpList {
        //{"ifand", {InstrParamType::STRING}},
        //{"ifor", {InstrParamType::STRING}},
        { "gotowarp", { InstrParamType::STRING } },
        { "set", { InstrParamType::STRING, InstrParamType::DOUBLE }, '=' },
        { "playmusique", { InstrParamType::STRING, InstrParamType::DOUBLE } },
        { "stopmusique", { InstrParamType::STRING } },
        { "playsound", { InstrParamType::STRING, InstrParamType::DOUBLE, InstrParamType::DOUBLE } },
        { "stopsound", { InstrParamType::STRING } },
        { "playsound3d", { InstrParamType::STRING, InstrParamType::DOUBLE, InstrParamType::DOUBLE, InstrParamType::DOUBLE } },
        { "stopsound3d", { InstrParamType::STRING } },
        { "setcursor", { InstrParamType::STRING, InstrParamType::STRING, InstrParamType::DOUBLE } },
        { "setcursordefault", { InstrParamType::DOUBLE, InstrParamType::STRING } },
        { "hidecursor", { InstrParamType::STRING, InstrParamType::DOUBLE } },
        { "setangle", { InstrParamType::DOUBLE, InstrParamType::DOUBLE } },
        { "interpolangle", { InstrParamType::DOUBLE, InstrParamType::DOUBLE, InstrParamType::DOUBLE } },
        { "anglexmax", { InstrParamType::DOUBLE } },
        { "angleymax", { InstrParamType::DOUBLE } },
        { "return", {} },
        { "end", {} },
        { "fade", { InstrParamType::DOUBLE, InstrParamType::DOUBLE, InstrParamType::DOUBLE } },
        { "lockkey", { InstrParamType::DOUBLE, InstrParamType::STRING } }, // Second argument is either a string or a number (0)
        { "resetlockkey", {} },
        { "setzoom", { InstrParamType::STRING } },
        { "gosub", { InstrParamType::STRING } },
        { "not", { InstrParamType::STRING } }
    };

    if (instructionName == "ifand" || instructionName == "ifor") {
        std::string line;
        if (!nextLine(line)) {
            std::cerr << m_currentLine << " Error: unexpected end of file" << std::endl;
            return false;
        }

        Lst::Instruction subInstruction;
        if (parsePlugin(line, subInstruction))
            ;
        else if (parseInstruction(line, subInstruction))
            ;
        else {
            // Error
            std::cerr << m_currentLine << " Error: unknown line in ifand/ifor: " << line << std::endl;
            return false;
        }

        std::vector<std::string> paramsList = split(params, ',');
        instruction.name = instructionName;
        for (const std::string& param : paramsList) {
            instruction.params.push_back(param);
        }
        instruction.subInstructions.push_back(subInstruction);

        return true;
    } else {
        for (const InstrTmp& instrTmp : instrTmpList) {
            if (instructionName == instrTmp.name) {
                instruction.name = instructionName;

                if (instrTmp.params.empty()) {
                    return true;
                }

                std::vector<std::string> paramsList = split(params, instrTmp.separator);
                if ((paramsList.size() != instrTmp.params.size())) {
                    std::cerr << m_currentLine << " Error: invalid " << instructionName << " instruction: " << params << std::endl;
                    return false;
                }

                for (size_t i = 0; i < paramsList.size(); ++i) {
                    if (instrTmp.params[i] == InstrParamType::DOUBLE) {
                        instruction.params.push_back((double)std::stoi(paramsList[i]));
                    } else if (instrTmp.params[i] == InstrParamType::DOUBLE) {
                        instruction.params.push_back(std::stod(paramsList[i]));
                    } else if (instrTmp.params[i] == InstrParamType::STRING) {
                        // Remove '.vr' if it exists
                        if (paramsList[i].find(".vr") != std::string::npos) {
                            paramsList[i] = paramsList[i].substr(0, paramsList[i].find(".vr"));
                        }

                        instruction.params.push_back(paramsList[i]);
                    }
                }

                return true;
            }
        }

        std::cerr << m_currentLine << " Error: unknown instruction: " << instructionName << std::endl;
        return false;
    }

    return true;
}

bool Lst::Impl::parsePluginInstruction(const std::string& line, Lst::Instruction& instruction)
{
    // Line format: funName(var1, var2, var3, ...)

    // Find the function name
    std::string funName = line.substr(0, line.find('('));
    trim(funName);

    instruction.name = funName;

    // Parse parameters
    std::string params = line.substr(line.find('(') + 1, line.find(')') - line.find('(') - 1);

    std::vector<std::string> paramsList = split(params, ',');
    for (const std::string& param : paramsList) {
        try {
            instruction.params.push_back(std::stod(param));
            continue;
        } catch (const std::invalid_argument&) {
            // Not a double: do nothing
        }

        // String
        std::string paramTmp = param;
        // Remove '"' if it exists
        if (param.front() == '"' && param.back() == '"') {
            paramTmp = paramTmp.substr(1, paramTmp.size() - 2);
        }

        // Replace '\' by '/'
        std::replace(paramTmp.begin(), paramTmp.end(), '\\', '/');

        instruction.params.push_back(paramTmp);
    }

    return true;
}

bool Lst::Impl::addVariable(const std::string& name)
{
    m_listVariables.insert(name);

    return true;
}

bool Lst::Impl::addInstruction(
    const std::string& warpName, const int& testId,
    const Instruction& instruction)
{
    // Register warp
    if (m_listWarps.empty()) {
        m_initWarp = warpName;
    }

    if (m_listWarps.find(warpName) == m_listWarps.end()) {
        Warp warp;
        warp.name = warpName;
        m_listWarps[warpName] = warp;
    }

    // Add instruction
    if (testId == -1) {
        m_listWarps[warpName].initBlock.push_back(instruction);
        return true;
    }

    // Register test
    if (m_listWarps[warpName].testBlockList.empty()) {
        m_listWarps[warpName].testBlockList[testId] = InstructionBlock();
    }

    if (testId < 0) {
        return false;
    }

    m_listWarps[warpName].testBlockList[testId].push_back(instruction);

    return true;
}

void optimizeInstructionBlock(Lst::InstructionBlock& block)
{
    for (auto it = block.begin(); it != block.end();) {
        // Combine identical ifand/ifor blocks
        if (it->name == "ifand" || it->name == "ifor") {
            if (it != block.end() - 1) {
                if ((it + 1)->name == it->name && (it + 1)->params == it->params) {
                    it->subInstructions.insert(
                        it->subInstructions.end(),
                        (it + 1)->subInstructions.begin(),
                        (it + 1)->subInstructions.end());
                    block.erase(it + 1);
                    continue;
                }
            }
        }
        ++it;
    }
}

void Lst::Impl::optimize()
{
    for (auto& warp : m_listWarps) {
        for (auto& test : warp.second.testBlockList) {
            optimizeInstructionBlock(test.second);
        }
    }

    for (auto& subroutine : m_listSubroutines) {
        optimizeInstructionBlock(subroutine.second.subInstructions);
    }

    for (auto& warp : m_listWarps) {
        optimizeInstructionBlock(warp.second.initBlock);
    }
}

/* PUBLIC */
Lst::Lst()
{
    d_ptr = new Impl;
}

Lst::~Lst()
{
    delete d_ptr;
}

bool Lst::parseLst(const std::string& fileName)
{
    d_ptr->m_file.open(fileName);
    if (!d_ptr->m_file) {
        std::cerr << "Could not open file: " << fileName << std::endl;
        return false;
    }

    std::string currentWarp;
    int currentTest = -2;

    d_ptr->m_currentLine = 0;
    std::string line;
    while (d_ptr->nextLine(line)) {
        std::string var;
        if (d_ptr->parseVariable(line, var)) {
            d_ptr->addVariable(var);
            continue;
        }

        if (d_ptr->parseWarp(line, currentWarp)) {
            continue;
        }

        if (d_ptr->parseTest(line, currentTest)) {
            if (currentWarp.empty()) {
                std::cerr << d_ptr->m_currentLine << " Error: [test] found before [warp]" << std::endl;
                return false;
            }

            continue;
        }

        Lst::Instruction instructionPlugin;
        if (d_ptr->parsePlugin(line, instructionPlugin)) {
            if (currentWarp.empty()) {
                std::cerr << d_ptr->m_currentLine << " Error: plugin found before [warp]" << std::endl;
                return false;
            }

            d_ptr->addInstruction(currentWarp, currentTest, instructionPlugin);
            continue;
        }

        Lst::Instruction instructionSubroutine;
        if (d_ptr->parseSubroutine(line, instructionSubroutine)) {
            d_ptr->m_listSubroutines[instructionSubroutine.name] = instructionSubroutine;
            continue;
        }

        Lst::Instruction instruction;
        if (d_ptr->parseInstruction(line, instruction)) {
            if (currentWarp.empty()) {
                std::cerr << d_ptr->m_currentLine << " Error: instruction found before [warp]" << std::endl;
                return false;
            }

            d_ptr->addInstruction(currentWarp, currentTest, instruction);
            continue;
        }

        std::cerr << d_ptr->m_currentLine << " Error: unknown line: " << line << std::endl;
        d_ptr->m_file.close();
        return false;
    }
    d_ptr->m_file.close();

    d_ptr->optimize();

    return true;
}

std::string doubleToString(double value)
{
    std::ostringstream oss;
    oss << std::setprecision(8) << std::noshowpoint << value;
    return oss.str();
}

std::string instructionToString(const Lst::Instruction& instruction, int level)
{
    std::string str;
    for (int i = 0; i < level; ++i) {
        str += "\t";
    }

    if (instruction.name == "ifand" || instruction.name == "ifor") {
        bool first = true;
        for (const auto& subInstruction : instruction.subInstructions) {
            if (first) {
                first = false;
            } else {
                for (int i = 0; i < level; ++i) {
                    str += "\t";
                }
            }
            str += instruction.name + "=";
            for (const auto& param : instruction.params) {
                str += std::get<std::string>(param);
                if (&param != &instruction.params.back()) {
                    str += ",";
                }
            }
            str += "\n";
            str += instructionToString(subInstruction, level + 1);
            if (&subInstruction != &instruction.subInstructions.back()) {
                str += "\n";
            }
        }

        return str;
    } else if (instruction.name == "plugin") {
        str += "plugin\n";
        for (const auto& subInstruction : instruction.subInstructions) {
            for (int i = 0; i < level + 1; ++i) {
                str += "\t";
            }
            str += subInstruction.name + "(";
            for (const auto& param : subInstruction.params) {
                if (std::holds_alternative<double>(param)) {
                    str += doubleToString(std::get<double>(param));
                } else {
                    str += std::get<std::string>(param);
                }

                if (&param != &subInstruction.params.back()) {
                    str += ",";
                }
            }
            str += ")\n";
        }

        for (int i = 0; i < level; ++i) {
            str += "\t";
        }
        str += "endplugin";
    } else if (instruction.name == "setzoom") {
        str += instruction.name + "=" + std::get<std::string>(instruction.params[0]);
    } else if (instruction.name == "gotowarp") {
        str += instruction.name + "=" + std::get<std::string>(instruction.params[0]) + ".vr";
    } else if (instruction.name == "set") {
        str += instruction.name + " "
            + std::get<std::string>(instruction.params[0]) + "="
            + doubleToString(std::get<double>(instruction.params[1]));
    } else {
        str += instruction.name;
        if (instruction.params.empty()) {
            return str;
        }
        str += " ";
        for (const auto& param : instruction.params) {
            if (std::holds_alternative<double>(param)) {
                str += doubleToString(std::get<double>(param));
            } else {
                str += std::get<std::string>(param);
            }

            if (&param != &instruction.params.back()) {
                str += ",";
            }
        }
    }

    return str;
}

void printWarp(std::ofstream& file, const std::string& warpName, const Warp& warp)
{
    file << "[warp]=" << warpName << ".vr," << warpName << ".tst" << std::endl;

    // Init block
    if (!warp.initBlock.empty()) {
        file << "\t[test]=-1" << std::endl;
        for (const auto& instruction : warp.initBlock) {
            file << instructionToString(instruction, 2) << std::endl;
        }
    }

    // Tests
    for (const auto& test : warp.testBlockList) {
        file << "\t[test]=" << test.first << std::endl;
        for (const auto& instruction : test.second) {
            file << instructionToString(instruction, 2) << std::endl;
        }
    }
}

bool Lst::saveLst(const std::string& fileName)
{
    std::ofstream file(fileName);
    if (!file) {
        std::cerr << "Could not open file: " << fileName << std::endl;
        return false;
    }

    // Variables
    for (const std::string& var : d_ptr->m_listVariables) {
        file << "[bool]=" << var << std::endl;
    }

    // Warps
    printWarp(file, d_ptr->m_initWarp, d_ptr->m_listWarps[d_ptr->m_initWarp]);
    for (const auto& warp : d_ptr->m_listWarps) {
        if (warp.first == d_ptr->m_initWarp) {
            continue;
        }

        printWarp(file, warp.first, warp.second);
    }

    // Subroutines
    for (const auto& subroutine : d_ptr->m_listSubroutines) {
        file << "label " << subroutine.first << std::endl;
        for (const auto& instruction : subroutine.second.subInstructions) {
            file << instructionToString(instruction, 1) << std::endl;
        }
        file << "return" << std::endl;
    }

    file.close();

    return true;
}

Lst::InstructionBlock& Lst::getInitBlock(const std::string& warpName)
{
    return d_ptr->m_listWarps[warpName].initBlock;
}

Lst::InstructionBlock& Lst::getTestBlock(
    const std::string& warpName, const int& testId)
{
    return d_ptr->m_listWarps[warpName].testBlockList[testId];
}

} // namespace ofnx::files
