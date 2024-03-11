#pragma once
#include <cstdint>
#include <fstream>
#include <vector>
#include <map>
#include <stdint.h>


using uchar = unsigned char;
using uint = unsigned int;

enum Modes {
    ASM = 0b1,
    BYTECODE = 0b10,
    BOTH = ASM | BYTECODE,
};

struct AsmParseContext {
    // bytecode file.
    std::fstream bcFile;
    std::vector<std::string> dontInclude;
    // jmpList[jmpMark][placesToInsert].
    // jmpList[jmpMark][0] = -1 по умолчанию - jmpMarkPtr для jmpMark.
    std::map<std::string, std::vector<int32_t>, std::less<>> jmpList;
    int byte;

    bool CreateJmpMark(const std::string_view& jmpMark, int bytePlaceInFile);
    void InsertJmpMark(const std::string_view& jmpMark, int bytePlaceInFile);
};
