#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdint.h>
#include <string_view>
#include <type_traits>
#include <vector>
#include <string>
#include <cstring>
#include "src/tables.h"
#include "src/macros.h"
#include "src/macro_fns.h"

namespace fs = std::filesystem;

// Посмотреть лимиты. Сейчас цифра взята с потолка.
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
// jmpList[jmpMark][placesToInsert].
// jmpList[jmpMark][0] = -1 по умолчанию - jmpMarkPtr для jmpMark.
std::map<std::string, std::vector<int32_t>, std::less<>> jmpList;
#if DEBUG
    bool debugMode = false;
#endif 

int VMachineMode();
int AsmParserMode();
void Init();
bool CreateJmpMark(const std::string_view&, int);
void InsertJmpMark(const std::string_view&, int);


int main(int argc, char** argv){
    Init();
    if (argc == 1){
        LOG("Try --help");
        return 0;
    }
    if (!strcmp("--help", argv[1])){
        LOG(HELPTEXT);
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
        #if DEBUG
            if (!strcmp(argv[i], "--debug")){
                debugMode = true;
                continue;
            }
        #endif

        if (!fs::exists(argv[i])){
            LOG("There is no \"" << argv[i] << "\" file or path");
            return 0;
        }
        if (nextIsOFile){
            nextIsOFile = false;
            if(!fs::is_directory(argv[i])){
                LOG('"' << argv[i] << "\" is not a directory (-o)");
                return 0;
            }
            outDir = argv[i];
        }
        else{
            if (!fs::is_regular_file(argv[i])){
                LOG('"' << argv[i] << "\" is not a file");
                return 0;
            }
            inpFiles.push_back(std::string(argv[i]));
        }
    }

    if (inpFiles.empty()){
        LOG("There is no input files");
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

// TODO переписать std::stoi на самописную с блекджеком и std::string_view.
// char, hex, oct, bin.
int32_t ParseValue(const std::string_view val, bool& error){
    error = 0;
    union{
        int32_t num;
        char parts[4];
    };

    if (val.size() > 2){

        if ( val[0] == '(' && val[val.size() - 1] == ')' ){
            int cur = 0;
            int leftBound = 0;
            num = 0;
            while (leftBound != val.size() - 1){
                int rightBound = val.find(' ', leftBound + 1);
                if (rightBound == -1){
                    rightBound = val.size() - 1;
                }
                std::string_view subVal(val.begin() + leftBound + 1, val.begin() + rightBound);
                leftBound = rightBound;
                union{
                    int32_t sNum;
                    char sParts[4];
                };
                sNum = ParseValue(subVal, error);
                if (error){
                    return 0;
                }
                int bytes = 4;
                for (int i = 3; i > 0; i--){
                    if (sParts[i] != 0){
                        break;
                    }
                    bytes--;
                }
                for(int i = 0; i < bytes; i++){
                    if (cur > 3){
                        error = true;
                        return 0;
                    }
                    parts[cur++] = sParts[i];
                }
            }
            for (int i = 0; i < 4; i++){
                int qwe = 0;
                (*(char*)&qwe) = parts[i];
            }
            return num;
        }

        if (val[0] == '"' && val[val.size() - 1] == '"' or (val[0] == '\'' && val[val.size() - 1] == '\'')){
            if(val.size() > 6){
                error = true;
                return 0;
            }
            for(int i = 1; i < val.size() - 1; i++){
                parts[i - 1] = val[i];
            }
            for(int i = val.size() - 2; i < 4; i++){
                parts[i] = 0;
            }
            return num;
        }
        
        try {
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
                    num = std::stoi((std::string)val);
                    return num;
            }
            num = std::stoi((std::string)val.substr(2), 0, base);
            return num;
        }
        catch (...) {
            error = 1;
            return 0;
        }
    }

    try {
        num = std::stoi((std::string)val);
        return num;
    }
    catch (...) {
        error = 1;
        return 0;
    }
}



int VMachineMode(){

    // Читаем программу.
    std::ifstream asmFile(inpFiles[0]);
    while (!asmFile.eof()){
        asmFile.read((char*)memory + REG_sptr, 1);
        REG_sptr++;
    }

    // Исполняем программу.
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
                if (memoryi(inn_next(1))){
                    REG_inn += 9;
                }
                else {
                    REG_inn = inn_next(5);
                }
                break;
            case 5:
                memoryi(REG_sptr) = memoryi(inn_next(1));
                REG_sptr += 4;
                REG_inn += 5;
                break;
            case 6:
                REG_sptr -= 4;
                memoryi(inn_next(1)) = memoryi(REG_sptr);
                REG_inn += 5;
                break;
            case 7:
                memoryi(REG_sptr) = REG_inn + 5;
                memoryi(REG_sptr + 4) = REG_fndt;
                REG_fndt = REG_sptr;
                REG_sptr += 8;
                REG_inn = inn_next(1);
                break;
            case 8:
                REG_inn = memoryi(REG_fndt);
                REG_sptr = REG_fndt;
                REG_fndt = memoryi(REG_sptr + 4);
                break;
            case 9:
                return inn_next(1);
            case 10:
                std::cin >> memory[memoryi(REG_inn + 1)];
                REG_inn += 5;
                break;
            case 11:
                LOG_STR(memory[REG_inn + 1]);
                REG_inn += 2;
                break;
            case 12:
                LOG_STR(memory[memoryi(REG_inn + 1)]);
                REG_inn += 5;
                break;
            case 13:
                memoryi(inn_next(1)) = memoryi(memoryi(inn_next(5)));
                REG_inn += 9;
                break;
            case 14:
                memoryi(memoryi(inn_next(1))) = memoryi(inn_next(5));
                REG_inn += 9;
                break;
            case 65:
                memoryi(inn_next(1)) = memoryi(inn_next(1)) + memoryi(inn_next(5));
                REG_inn += 9;
                break;
            case 66:
                memoryi(inn_next(1)) = memoryi(inn_next(1)) - memoryi(inn_next(5));
                REG_inn += 9;
                break;
            case 67:
                memoryi(inn_next(1)) = memoryi(inn_next(1)) / memoryi(inn_next(5));
                REG_inn += 9;
                break;
            case 68:
                memoryi(inn_next(1)) = memoryi(inn_next(1)) % memoryi(inn_next(5));
                REG_inn += 9;
                break;
            case 69:
                memoryi(inn_next(1)) = memoryi(inn_next(1)) * memoryi(inn_next(5));
                REG_inn += 9;
                break;
            case 70:
                memoryi(inn_next(1)) = memoryi(inn_next(1)) == memoryi(inn_next(5));
                REG_inn += 9;
                break;
            case 71:
                memoryi(inn_next(1)) = memoryi(inn_next(1)) < memoryi(inn_next(5));
                REG_inn += 9;
                break;
            case 72:
                memoryi(inn_next(1)) = memoryi(inn_next(1)) > memoryi(inn_next(5));
                REG_inn += 9;
                break;
            case 73:
                memoryi(inn_next(1)) = memoryi(inn_next(1)) && memoryi(inn_next(5));
                REG_inn += 9;
                break;
            case 74:
                memoryi(inn_next(1)) = memoryi(inn_next(1)) || memoryi(inn_next(5));
                REG_inn += 9;
                break;
            case 75:
                memoryi(inn_next(1)) = !memoryi(inn_next(1));
                REG_inn += 5;
                break;
            case 76:
                memoryi(inn_next(1)) = memoryi(inn_next(1)) ^ memoryi(inn_next(5));
                REG_inn += 9;
                break;
            case 77:
                memoryi(inn_next(1)) = memoryi(inn_next(1)) & memoryi(inn_next(5));
                REG_inn += 9;
                break;
            case 78:
                memoryi(inn_next(1)) = memoryi(inn_next(1)) | memoryi(inn_next(5));
                REG_inn += 9;
                break;
            case 79:
                memoryi(inn_next(1)) = ~memoryi(inn_next(1));
                REG_inn += 5;
                break;
            case 80:
                memoryi(inn_next(1)) = memoryi(inn_next(1)) << memoryi(inn_next(5));
                REG_inn += 9;
                break;
            case 81:
                memoryi(inn_next(1)) = memoryi(inn_next(1)) >> memoryi(inn_next(5));
                REG_inn += 9;
                break;
        }
    }
    return 1;
}



int AsmParserMode(){
    for(std::string& asmFileName : inpFiles){
        LOG(" > Processing " << asmFileName);
        std::ifstream asmFile(asmFileName);
        // Теперь читаем и пробелы.
        asmFile.unsetf(std::ios::skipws);
        {
            auto dot = asmFileName.find_last_of('.');
            if (dot != -1){
                asmFileName.resize(dot);
            }
            asmFileName += ".shbyte";
        }
        // bytecode file.
        std::ofstream bcFile(asmFileName, std::ios::binary);        
        std::string token;
        std::vector<uint8_t>* commandInfo;
        std::string_view jmpMark;
        // шагов до следующей команды.
        int stepsToNext = 0;
        int line = 1;
        int byte = 0;
        bool isQuotes = false;
        bool isBrackets = false;
        int32_t insA = 0;

    cycle1:

        // Собираем токен.
        while(true){
            char let;
            asmFile >> let;
            if(asmFile.eof()){
                break;
            }
            switch (let){
                case ' ':
                    if(isQuotes || isBrackets){
                        token.push_back(let);
                        break;
                    }
                    goto fullToken;
                case '\n':
                newLineProc:
                    if(isQuotes){
                        ERROR("quotes" << " are not closed on line " << line << '.');
                    }
                    if(isBrackets){
                        ERROR("brackets" << " are not closed on line " << line << '.');
                    }
                    line++;
                    bcFile.flush();
                    goto fullToken;
                case '\'':
                case '\"':
                    token.push_back(let);
                    isQuotes = !isQuotes;
                    if (!isQuotes && !isBrackets){
                        goto fullToken;
                    }
                    break;
                case '(':
                    if(isQuotes){
                        token.push_back(let);
                        break;
                    }
                    if (isBrackets){
                        ERROR("double brackets on line " << line << '.')
                    }
                    token.push_back(let);
                    isBrackets = true;
                    break;
                case ')':
                    if(isQuotes){
                        token.push_back(let);
                        break;
                    }
                    if (isBrackets){
                        token.push_back(let);
                        isBrackets = false;
                        goto fullToken;
                    }
                    ERROR("no brackets to close by ')' on line " << line << '.')
                case ';':
                    if(isQuotes){
                        token.push_back(let);
                        break;
                    }
                    while(true){
                        asmFile >> let;
                        if(asmFile.eof()){
                            break;
                        }
                        if(let == '\n'){
                            goto newLineProc;
                        }
                    }
                    goto fullToken;
                default:
                    token.push_back(let);
            }
        }
    fullToken:
        if (token.empty()){
            goto tokenProcEnd;
        }

        // jmp
        if(token[0] == ':' && token[token.size() - 1] == ':'){
            // TODO кажется, тут UB, т.к. string_view ссылается на строку после отчистки.
            jmpMark = {token.begin() + 1, token.end() - 1};
            token.clear();
        }
        // ~jmp

        if (stepsToNext == 0){
            // Если token = новая команда (не значение).
            
            // jmp
            if (!jmpMark.empty()){
                bool error = CreateJmpMark(jmpMark, REGMEMAMOUNT + byte);
                if (error){
                    ERROR("line " << line << ": jmp mark \"" << jmpMark << "\" already defined.");
                }
                jmpMark = {};
                goto tokenProcEnd;
            }
            // ~jmp
            
            // ins
            if (token == "ins"){
                insA = -1;
                stepsToNext = 2;
                token.clear();
                goto tokenProcEnd;
            }
            // ~ins

            auto it = asmTable.find(token);
            if (it == asmTable.end()){
                ERROR("wrong asm command \"" << token << "\" on line " << line << '.');
            }
            commandInfo = &(it->second);
            bcFile << (char)((*commandInfo)[0]);
            stepsToNext = commandInfo->size() - 1;
            token.clear();
            #if DEBUG
                if (debugMode){
                    bcFile << ':';
                }
            #endif
            byte++;
        }
        else {
            // Если token = значение (не команда)

            // ins
            if (insA == -1){
                // Первый аргумент ins.
                // Считываем размер следующего аргумета в байтах.
                if (!jmpMark.empty()){
                    ERROR("line " << line << ". Jmp mark cannot be a size of a value.");
                }
                bool error;
                insA = ParseValue(token, error);
                if (error){
                    ERROR("wrong value format on line " << line << '.');
                }
                if (insA <= 0){
                    ERROR("first argument <= 0 on line " << line << '.');
                }
                if (insA > 4){
                    ERROR("values >4 Bytes is not suported yet. Line " << line << '.');
                }
                stepsToNext--;
                token.clear();
                goto tokenProcEnd;
            }
            if (insA){
                // Второй аргумент ins.
                bool error;
                union{
                    int32_t val;
                    uint8_t parts[4];
                };

                // jmp
                if (jmpMark.empty()){
                    val = ParseValue(token, error);
                }
                else if (insA == 4){
                    InsertJmpMark(jmpMark, byte);
                    bcFile.seekp(4, std::ios::cur);
                    byte += 4;
                    jmpMark = {};
                    insA = 0;
                    stepsToNext--;
                    goto tokenProcEnd;
                }
                else {
                    ERROR("not enough bytes in the argument to store a jmp mark :" << jmpMark << ": on line " << line << '.');
                }
                // ~jmp

                if (error){
                    ERROR("wrong value \"" << token << "\" on line " << line << '.');
                }
                for (int i = 0; i < insA; i++){
                    bcFile << parts[i];
                }
                byte += insA;
                insA = 0;
                stepsToNext--;
                token.clear();
                goto tokenProcEnd;
            }
            // ~ins
            

            int bytes = (*commandInfo)[commandInfo->size() - stepsToNext];
            if (bytes > 4){
                ERROR("values >4 Bytes is not suported yet. Line " << line << '.');
            }
            // jmp
            if (!jmpMark.empty()){
                if (bytes != 4){
                    ERROR("not enough bytes in the argument to store a jmp mark :" << jmpMark << ": on line " << line << '.');
                }
                InsertJmpMark(jmpMark, byte);
                bcFile.seekp(4, std::ios::cur);
                byte += 4;
                stepsToNext--;
                jmpMark = {};
                goto tokenProcEnd;
            }
            // ~jmp

            union{
                int num;
                char parts[4];
            };
            auto it = regTable.find(token);
            if (it != regTable.end()){
                num = it->second;
                goto regSkip1;
            }
            {
                bool error;
                num = ParseValue(token, error);
                if(error){
                    ERROR("wrong value \"" << token << "\" on line " << line << '.');
                }
            }
            if (bytes == 1){
                if (num >= 256){
                    WARNING("byte overflow on line " << line << ". Value \"" << token << "\" as a number \"" << (int)parts[0] << "\".");
                }
            }
        regSkip1:
            for (int i = 0; i < bytes; i++){
                bcFile << parts[i];
            }

            stepsToNext--;
            token.clear();
            #if DEBUG
                if (debugMode){
                    if (stepsToNext == 0){
                        bcFile << '_';
                    }else{
                        bcFile << '.';
                    }
                }
            #endif
            byte += bytes;
        }

    tokenProcEnd:
        if (!asmFile.eof()){
            goto cycle1;
        }
        
        if (stepsToNext != 0){
            ERROR("the last command do not have enough arguments");
        }

        // jpm
        for (auto& [jmpName, placesToInsert] : jmpList){
            union{
                int32_t val;
                uint8_t parts[4];
            };
            val = placesToInsert[0];
            if (val == -1){
                ERROR(TERMCOLOR::FG_LBLUE << ':' << jmpName << ":" << TERMCOLOR::LOG_DEFAULT << " is not defined.")
            }
            for (int j = 1; j < placesToInsert.size(); j++){
                bcFile.seekp(placesToInsert[j], std::ios::beg);
                bcFile << parts[0] << parts[1] << parts[2] << parts[3];
            }
        }
        // ~jmp

        bcFile.close();
        LOG(" > Done " << asmFileName);

    }
    return 0;
}

void Init(){
    // DLog
    #if _DEBUG && DLOGISFILE
        __dLogFile.file.open(DLOGFILEPATH, std::ios::out | std::ios::trunc);
    #endif
}

// return 1 если jmpMark уже определена.
bool CreateJmpMark(const std::string_view& jmpMark, int bytePlaceInBytecode){
    auto it = jmpList.find(jmpMark);
    if (it == jmpList.end()){
        jmpList.insert({ (std::string)jmpMark, {bytePlaceInBytecode} });
    }
    else {
        if (it->second[0] != -1){
            return 1;
        }
        it->second[0] = bytePlaceInBytecode;
    }
    return 0;
}

void InsertJmpMark(const std::string_view& jmpMark, int bytePlaceInFile){
    auto it = jmpList.find(jmpMark);
    if (it == jmpList.end()){
        jmpList.insert({ (std::string)jmpMark, {-1, bytePlaceInFile} });
    }
    else{
        it->second.push_back(bytePlaceInFile);
    }
}