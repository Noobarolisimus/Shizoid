#pragma once
#include <vector>
#include "config.h"
#include "tables.h"


#define inn_next(bytes) (*(int32_t*)(memory + REG_inn + bytes))
#define memoryi(bytes) (*(int32_t*)(memory + bytes))

// inline int32_t& inn_next(int bytes){
//     return *(int32_t*)(memory + REG_inn + bytes);
// }
// inline int32_t& memoryi(int bytes){
//     return *(int32_t*)(memory + bytes);
// }


void AppExit(int code);

int ParseArgs(int argc, char** argv);

void GetOutPath(const fs::path& asmPath, fs::path& outBytecodePath);

uint8_t ParseEscChar(uint8_t character);

// Выбрасывает 0xff, если digit неправильный
inline unsigned int HexToInt(uint8_t digit){
    if (std::isdigit((uint8_t)digit)){
        return digit - '0';
    }
    if (digit >= 'a' && digit <= 'f'){
        return digit - 'a' + 10;
    }
    if (digit >= 'A' && digit <= 'F'){
        return digit - 'A' + 10;
    }
    return 0xff;
}


// void LongAdd(uint8_t a, std::vector<uint8_t>& b, int bytePlaceToAdd){
//     
// }

// TODO самописный std::stoi
// dec - 4 Bytes.
// bin, oct, hex - unlimited.
void ParseNum(std::string_view val, std::vector<uint8_t>& outBytes, bool& error);

// TODO переписать std::stoi на самописную с блекджеком и std::string_view.
// char, hex, oct, bin.
void ParseValue(const std::string_view val, std::vector<uint8_t>& outBytes, bool& error);
