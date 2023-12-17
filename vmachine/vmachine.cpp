#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <cstring>
#include "src/tables.h"
//#include "src/macros.h"
//#include "src/macro_fns.h"

namespace fs = std::filesystem;

// Посмотреть лимиты. Сейчас цифра взята с потолка
uint8_t memory[(int)4e6];

// #define inn_next(bytes) (*(int32_t*)(memory + REG_inn + bytes))
// #define memoryi(bytes) (*(int32_t*)(memory + bytes))

inline int32_t& inn_next(int bytes){
    return *(int32_t*)(memory + REG_inn + bytes);
}

inline int32_t& memoryi(int bytes){
    return *(int32_t*)(memory + bytes);
}

enum class Modes {
    ASM = 1,
    BYTECODE,
    BOTH,
} mode;
bool duplicates = false;
bool nextIsOFile = false;
std::vector<std::string> inpFiles;
std::string outDir;

int VMachineMode();
int AsmParserMode();
void Init();


int main(int argc, char** argv){
    Init();
    if (argc == 1){
        std::cout << "Try --help\n";
        return 0;
    }
    // if (argc > 2){
    //     std::cout << "Too many arguments. Pass a shizoid bytecode file path (.shbyte)\n";
    //     return 0;
    // }
    if (!strcmp("--help", argv[1])){
        std::cout << HELPTEXT;
        return 0;
    }

    for(int i = 1; i < argc; i++){
        if (!strcmp(argv[i], "--asm")){
            mode = Modes::ASM;
            continue;
        }
        if (!strcmp(argv[i], "--bytecode")){
            mode = Modes::BYTECODE;
            continue;
        }
        if (!strcmp(argv[i], "--both")){
            mode = Modes::BOTH;
            continue;
        }
        if (!strcmp(argv[i], "-o")){
            nextIsOFile = true;
            continue;
        }

        if (!fs::exists(argv[i])){
            std::cout << "There is no \"" << argv[i] << "\" file or path";
            return 0;
        }
        if (nextIsOFile){
            nextIsOFile = false;
            if(!fs::is_directory(argv[i])){
                std::cout << '"' << argv[i] << "\" is not a directory (-o)";
                return 0;
            }
            outDir = argv[i];
        }
        else{
            if (!fs::is_regular_file(argv[i])){
                std::cout << '"' << argv[i] << "\" is not a file";
                return 0;
            }
            inpFiles.push_back(std::string(argv[i]));
        }
    }

    if (inpFiles.empty()){
        std::cout << "There is no input files";
        return 0;
    }

    std::fill(memory, memory + REGMEMAMOUNT, 0);
    REG_inn = REGMEMAMOUNT;
    REG_sptr = REGMEMAMOUNT;

    

    int res = 0;

    if (mode != Modes::BYTECODE){
        if (outDir.empty()){
            outDir = fs::current_path().generic_string();
        }
        res = AsmParserMode();
    }
    if (res != 0){
        return res;
    }

    if (mode != Modes::ASM)
        res = VMachineMode();
    return res;
}

// char, hex, oct, bin
int32_t ParseValue(const std::string& val, bool& error){
    error = false;
    if (val.size() > 2){
        if (val[0] == '"' && val[val.size() - 1] == '"' or (val[0] == '\'' && val[val.size() - 1] == '\'')){
            if(val.size() > 6){
                error = true;
                return 0;
            }
            union{
                int32_t out;
                char parts[4];
            };
            for(int i = 1; i < val.size() - 1; i++){
                parts[i - 1] = val[i];
            }
            for(int i = val.size() - 2; i < 4; i++){
                parts[i] = 0;
            }
            return out;
        }
        
        int base;
        switch (val[1]){
            case 'b':
                base = 2;
                break;
            case 'o':
                base = 8;
                break;
            case 'x':
                base = 16;
                break;
            default:
                return std::stoi(val);
        }
        return std::stoi(val.c_str() + 2, 0, base);
    }
    return std::stoi(val);
}

int VMachineMode(){

    // Читаем программу
    std::ifstream asmFile(inpFiles[0]);
    while (!asmFile.eof()){
        asmFile.read((char*)memory + REG_sptr, 1);
        REG_sptr++;
    }
    
    // Исполняем программу
    while (1){
        switch (memory[REG_inn]) {
            default:
            case 0:
                REG_inn++;
                break;
            case 1:
                memoryi(inn_next(1)) = inn_next(5);
                REG_inn += 9;
                break;
            case 2:
                REG_inn = inn_next(1);
                break;
            case 3:
                memoryi(inn_next(1)) = memoryi(inn_next(5));
                REG_inn += 9;
                break;
            case 4:
                if (inn_next(1)){
                    REG_inn += 5;
                }
                else {
                    REG_inn = inn_next(1);
                }
                break;
            case 5:
                REG_sptr += inn_next(1);
                REG_inn += 5;
                break;
            case 6:
                REG_sptr -= inn_next(1);
                REG_inn += 5;
                break;
            case 7:
            {
                int32_t old_fndt = REG_fndt;
                REG_fndt = REG_sptr;
                REG_sptr += memoryi(inn_next(1));
                memoryi(REG_sptr) = old_fndt;
                REG_sptr += 4;
                REG_rinf = REG_inn + 5;
                REG_inn = inn_next(1) + 4;
            }
                break;
            case 8:
            {
                REG_inn = REG_rinf;
                int32_t temp = REG_sptr;
                REG_sptr = REG_fndt;
                REG_fndt = memoryi(temp - 4);
            }
                break;
            case 9:
                std::cin >> memory[memoryi(REG_inn + 1)];
                REG_inn += 5;
                break;
            case 10:
                std::cout << memory[REG_inn + 1];
                REG_inn += 2;
                break;
            case 11:
                std::cout << memory[memoryi(REG_inn + 1)];
                REG_inn += 5;
                break;
            case 12:
                return inn_next(1);

        }
    }
    return 1;
}

int AsmParserMode(){
    for(std::string& asmFileName : inpFiles){
        std::cout << " > Processing " << asmFileName << "\n";
        std::ifstream asmFile(asmFileName);
        // Теперь читаем и пробелы
        asmFile.unsetf(std::ios::skipws);
        {
            auto dot = asmFileName.find_last_of('.');
            if (dot != -1){
                asmFileName.resize(dot);
            }
            asmFileName += ".shbyte";
        }
        // bytecode file
        std::ofstream bcFile(asmFileName, std::ios::binary);
        std::string token;
        std::vector<uint8_t>* commandInfo;
        // шагов до следующей команды
        int stepsToNext = 0;
        int line = 1;

    cycle1:

        while(!asmFile.eof()){
            char let;
            asmFile >> let;
            switch (let){
                case '\n':
                    line++;
                case ' ':
                    goto fullToken;
                case ';':
                    while(!asmFile.eof()){
                        asmFile >> let;
                        if(let == '\n'){
                            line++;
                            goto fullToken;
                        }
                    }
                    break;
                default:
                    token.push_back(let);
            }
        }
    fullToken:
        // Если новая команда (не значение)
        if (stepsToNext == 0){
            auto it = asmTable.find(token);
            if(it == asmTable.end()){
                std::cout << "ERROR: Wrong asm command \"" << token << "\" on line " << line << '.';
                return 1;
            }
            commandInfo = &(it->second);
            bcFile.flush();
            bcFile << (char)((*commandInfo)[0]);
            bcFile.flush();
            stepsToNext = commandInfo->size() - 1;
            token.clear();
        }
        else {
            int bytes = (*commandInfo)[commandInfo->size() - stepsToNext];
            if (bytes > 4){
                std::cout << "ERROR: values >4 Bytes is not suported yet. Line " << line << '.';
            }
            union{
                int num;
                char parts[4];
            };
            auto it = regTable.find(token);
            if (it != regTable.end()){
                num = it->second;
                goto regskip1;
            }
            {
                bool error;
                num = ParseValue(token, error);
                if(error){
                    std::cout << "ERROR: wrong value \"" << token << "\" on line " << line << '.';
                }
            }
            if (bytes == 1){
                if (num >= 256){
                    std::cout << "Warning: byte overflow on line " << line << ". Value \"" << token << "\" as number \"" << (int)parts[0] << "\".\n";
                }
            }
        regskip1:
            for (int i = 0; i < bytes; i++){
                bcFile << parts[i];
                bcFile.flush();
            }

            stepsToNext--;
            token.clear();
        }
        if (!asmFile.eof()){
            goto cycle1;
        }
        
        if (stepsToNext != 0){
            std::cout << "ERROR: the last command do not have enough arguments";
            return 1;
        }
        std::cout << " > Done " << asmFileName << "\n";

    }
    return 0;
}

void Init(){
    // DLog
    #if _DEBUG && DLOGISFILE
        __dLogFile.file.open(DLOGFILEPATH, std::ios::out | std::ios::trunc);
    #endif
}