#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iomanip>
#include <ios>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdint.h>
#include <string_view>
#include <type_traits>
#include <vector>
#include <string>
#include <cstring>
#include <chrono>
#include <bit>
#include <ranges>
#include "src/tables.h"
#include "src/macros.h"
#include "src/macro_fns.h"

namespace fs = std::filesystem;
namespace chrono = std::chrono;
namespace views = std::views;
using ios = std::ios;

// TODO Посмотреть лимиты. Сейчас цифра взята с потолка.
uint8_t memory[(int)4e6];

// #define inn_next(bytes) (*(int32_t*)(memory + REG_inn + bytes))
// #define memoryi(bytes) (*(int32_t*)(memory + bytes))

inline int32_t& inn_next(int bytes){
    return *(int32_t*)(memory + REG_inn + bytes);
}

inline int32_t& memoryi(int bytes){
    return *(int32_t*)(memory + bytes);
}

enum Modes {
    ASM = 0b1,
    BYTECODE = 0b10,
    BOTH = ASM | BYTECODE,
} mode;


namespace ArgVars{
    bool nextIsOFile = false;
    std::vector<std::string> inpFiles;
    // TODO нигде не используется, что забавно.
    std::string outDir;
    bool printExitInfo = false;
    #if DEBUG
        bool debugMode = false;
    #endif 
}

int VMachineMode();
int AsmParserMode();
void Init();
// bool CreateJmpMark(const std::string_view&, int);
// void InsertJmpMark(const std::string_view&, int);
int ParseArgs(int, char**);

void AppExit(int code){
    SETCOLOR(TERMCOLOR::DEFAULT);
    exit(code);
}


int main(int argc, char** argv){
    Init();

    if (int error = ParseArgs(argc, argv); error){
        return 0;
    }

    std::fill(memory, memory + REGMEMAMOUNT, 0);
    REG_inn = REGMEMAMOUNT;
    REG_sptr = REGMEMAMOUNT;

    int error = 0;

    if (mode & Modes::ASM){
        if (ArgVars::outDir.empty()){
            ArgVars::outDir = fs::current_path().generic_string();
        }
        error = AsmParserMode();
        
    }
    if (error != 0){
        AppExit(error);
    }

    if (mode & Modes::BYTECODE)
        error = VMachineMode();
    AppExit(error);
}


uint8_t ParseEscChar(uint8_t character){
    switch (character){
        case 'n': return '\n';
        case '\'': return '\'';
        case '\"': return '\"';
        case '0': return '\0';
        case '\\': return '\\';
        case 't': return '\t';
    }
    // TODO? Заменить на std::optional
    return character;
}

// TODO
int32_t BytesToInt(const std::vector<uint8_t> bytes);

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
void ParseNum(std::string_view val, std::vector<uint8_t>& outBytes, bool& error){
    static_assert(std::endian::native == std::endian::little, "Oopsy, only littleEndian is supported.");
    if (val.size() >= 2 && !std::isdigit((uint8_t)val[1])){
        int baseShift;
        switch (val[1]){
        case 'b':
            baseShift = 1;
            break;
        case 'o':
            baseShift = 3;
            break;
        case 'x':
            baseShift = 4;
            break;
        default:
            error = true;
            return;
        }
        int bitCur = outBytes.size() * 8;
        outBytes.insert(outBytes.end(), {0, 0});
        val.remove_prefix(2);
        for (uint8_t let : val | views::reverse){
            int byteCur = bitCur / 8;
            if (byteCur + 1 >= outBytes.size()){
                outBytes.push_back(0);
            }
            unsigned int bits = HexToInt(let);
            if (bits == 0xff){
                error = true;
                return;
            }
            // TODO! Возможно сложение заходит на память вне outBytes.
            // Хотя там могут прибавляться только нули, так что ничего плохого случиться не должно.
            *(uint32_t*)(&outBytes[byteCur]) += bits << (bitCur - byteCur * 8);
            bitCur += baseShift;
        }
        goto parseNum_zeros;
    }
    outBytes.insert(outBytes.end(), {0, 0, 0, 0});
    size_t cur;
    // TODO Переписать std::stoi
    *(int32_t*)(&outBytes[outBytes.size() - 4]) = std::stoi((const std::string)(val), &cur);
    if (cur != val.size()){
        error = true;
        return;
    }
parseNum_zeros:
    for (int i = outBytes.size() -1; i > 0 && outBytes[i] == 0; i--){
        outBytes.pop_back();
    }
    return;
}


// TODO переписать std::stoi на самописную с блекджеком и std::string_view.
// char, hex, oct, bin.
void ParseValue(const std::string_view val, std::vector<uint8_t>& outBytes, bool& error){
    error = 0;
    if (val.size() > 2){
        if (val[0] == '(' && val[val.size() - 1] == ')'){
            int leftBound = 0;
            while (leftBound != val.size() - 1){
                int rightBound = val.find(' ', leftBound + 1);
                if (rightBound == -1){
                    rightBound = val.size() - 1;
                }
                std::string_view subVal(val.begin() + leftBound + 1, val.begin() + rightBound);
                leftBound = rightBound;
                ParseValue(subVal, outBytes, error);
                if (error){
                    return;
                }
            }
            return;
        }

        if (val[0] == '"' && val[val.size() - 1] == '"'){
            // TODO? Возможно стоит находить все экранированые символы и учитывать это.
            outBytes.reserve(outBytes.size() + val.size());
            for (int i = 1; i < val.size() - 1; i++){
                if (val[i] == '\\'){
                    if (i > val.size() - 3){
                        error = true;
                        return;
                    }
                    outBytes.push_back(ParseEscChar(val[i + 1]));
                    i++;
                    continue;
                }
                outBytes.push_back(val[i]);
            }
            return;
        }
    }
    ParseNum(val, outBytes, error);
}


int ParseArgs(int argc, char** argv){
    using namespace ArgVars;
    if (argc == 1){
        LOG("Try --help");
        return 1;
    }
    if (!strcmp("--help", argv[1])){
        LOG(HELPTEXT);
        return 1;
    }

    for(int i = 1; i < argc; i++){
        if (strncmp(argv[i], "--", 2) == 0){
            char* arg = argv[i] + 2;
            if (strcmp(arg, "asm") == 0){
                mode = Modes::ASM;
                continue;
            }
            if (strcmp(arg, "bytecode") == 0){
                mode = Modes::BYTECODE;
                continue;
            }
            if (strcmp(arg, "both") == 0){
                mode = Modes::BOTH;
                continue;
            }
            if (strcmp(arg, "exitinfo") == 0){
                printExitInfo = true;
                continue;
            }
            #if DEBUG
                if (strcmp(arg, "debug") == 0){
                    debugMode = true;
                    continue;
                }
            #endif
            LOG("Unknown arg \"" << argv[i] << "\"")
            return 1;
        }
        
        if (*argv[i] == '-'){
            char arg = *(argv[i] + 1);
            switch (arg){
            case 'o':
                nextIsOFile = true;
                continue;
            }
            LOG("Unknown arg \"" << argv[i] << "\"")
            return 1;
        }

        if (!fs::exists(argv[i])){
            LOG("There is no \"" << argv[i] << "\" file or path");
            return 1;
        }
        if (nextIsOFile){
            nextIsOFile = false;
            if(!fs::is_directory(argv[i])){
                LOG('"' << argv[i] << "\" is not a directory (-o)");
                return 1;
            }
            outDir = argv[i];
        }
        else{
            if (!fs::is_regular_file(argv[i])){
                LOG('"' << argv[i] << "\" is not a file");
                return 1;
            }
            inpFiles.push_back(std::string(argv[i]));
        }
    }

    if (inpFiles.empty()){
        LOG("There is no input files");
        return 1;
    }
    return 0;
}



int VMachineMode(){
    if (mode == Modes::BOTH){
        auto dot = ArgVars::inpFiles[0].find_last_of('.');
        if (dot != -1){
            ArgVars::inpFiles[0].resize(dot);
        }
        ArgVars::inpFiles[0] += ".shbyte";
    }

    // Читаем программу.
    std::ifstream asmFile(ArgVars::inpFiles[0]);
    
    while (!asmFile.eof()){
        asmFile.read((char*)memory + REG_sptr, 1);
        REG_sptr++;
    }

    auto startTime = chrono::steady_clock::now();

    // Исполняем программу.
    while (1){
        switch (memory[REG_inn]) {
        default:
        case 0:
            goto vMachineModeEnding;
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
            REG_inn++;
        case 10:
            std::cin >> memory[memoryi(REG_inn + 1)];
            REG_inn += 5;
            break;
        case 11:
            LOG_STR(memory[REG_inn + 1]);
            REG_inn += 2;
            // TODO! Временно
            std::cout.flush();
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
        case 15:
            REG_inn += inn_next(1);
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
            memoryi(inn_next(1)) = memoryi(inn_next(1)) <= memoryi(inn_next(5));
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
vMachineModeEnding: 
    if (ArgVars::printExitInfo) {
        int executionTime = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - startTime).count();
        LOG("\nProgram finished in " << std::fixed << std::setprecision(6) << executionTime / 1000000.f << " seconds with code " << inn_next(1));
    }
    return inn_next(1);
}


// TODO Вынести в файл (+ методы в конце vmachine.cpp).
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

// 0 - успешный парсинг.
// 1 - отмена парсинга, файл встречен не впервый раз (#once).
int ParseAsm(const std::string_view& asmFileName, AsmParseContext& ctx, unsigned int depth = 1){
    {
        auto result = std::find(ctx.dontInclude.begin(), ctx.dontInclude.end(), asmFileName);
        if (result != ctx.dontInclude.end()){
            return 1;
        }
    }
    // TODO медленно из-за постоянного переключения цвета ? 
    LOG_STR(" ");
    for (int i = 0; i < depth; i++){
        LOG_STR(">");
    }
    LOG(" " << TERMCOLOR::FG_CYAN << "Processing " << TERMCOLOR::LOG_DEFAULT << asmFileName);
    std::ifstream asmFile(asmFileName);
    // Читаем пробелы.
    asmFile.unsetf(ios::skipws);
    std::string token;
    std::string lastInstruction;
    std::vector<uint8_t>* commandInfo;
    std::string_view jmpMark;
    // шагов до следующей команды.
    int stepsToNext = 0;
    int line = 1;
    bool isQuotes = false;
    bool isBrackets = false;
    // Да, плохо, но:
    //  -1 - ничего.
    //  -2 - когда нашли ins.
    // >=0 - когда нашли A.
    int32_t insA = -1;

    
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
                goto fullToken;
            case '\"':
                token.push_back(let);
                {
                    int isOdd = 0;
                    for (int i = token.size() - 2; i >= 0; i--){
                        if (token[i] != '\\')
                            break;
                        isOdd ^= 1;
                    }
                    if (isOdd){
                        break;
                    }
                }
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

    // #
    if (token[0] == '#'){
        const char* str = token.c_str() + 1;
        if(stepsToNext != 0 || insA != -1){
            ERROR("A command do not has enough args on line " << line - 1 << ".");
        }
        if (strcmp(str, "include") == 0){
            
            std::string fileName;
            // TODO Cократить путь.
            // TODO Сделать поиск по директориям и т.п. (=сделать умным).
            // TODO (sub) Искать от inp файла
            char let;
            while (true){
                asmFile >> let;
                if (let == '\n'){
                    break;
                }
                fileName.push_back(let);
                if (asmFile.eof()){
                    break;
                }
            }
            if (!fs::exists(fileName)){
                ERROR("line " << line << ": file \"" << TERMCOLOR::FG_BLUE << fileName << TERMCOLOR::LOG_DEFAULT << "\" does not exist.")
            }
            if (!fs::is_regular_file(fileName)){
                ERROR("line " << line << ": file \"" << TERMCOLOR::FG_BLUE << fileName << TERMCOLOR::LOG_DEFAULT << "\" is not a regular.")
            }

            if(ParseAsm(fileName, ctx, depth + 1) == 0){
                // TODO? медленно из-за постоянного переключения цвета? 
                LOG_STR(" ");
                for (int i = -1; i < (int)depth; i++){
                    LOG_STR(">");
                }
                LOG(" " << TERMCOLOR::FG_LGREEN << "Done " << TERMCOLOR::LOG_DEFAULT << fileName);
            }
        }
        else if (strcmp(str, "once") == 0){
            ctx.dontInclude.push_back((std::string)asmFileName);
        }
        line++;
        token.clear();
        goto tokenProcEnd;
    }
    // ~#

    // jmp
    if(token[0] == ':' && token[token.size() - 1] == ':'){
        // TODO! Кажется, тут UB, т.к. string_view ссылается на строку после отчистки.
        jmpMark = {token.begin() + 1, token.end() - 1};
        token.clear();
    }
    // ~jmp

    if (stepsToNext == 0){
        // Если token == новая команда (не значение).
        
        // jmp
        if (!jmpMark.empty()){
            bool error = ctx.CreateJmpMark(jmpMark, ctx.byte);
            if (error){
                ERROR("line " << line << ": jmp mark \"" << TERMCOLOR::FG_LBLUE << ':' << jmpMark << ':' << TERMCOLOR::LOG_DEFAULT << "\" already defined.");
            }
            jmpMark = {};
            goto tokenProcEnd;
        }
        // ~jmp
        
        // ins
        if (token == "ins"){
            insA = -2;
            stepsToNext = 2;
            token.clear();
            goto tokenProcEnd;
        }
        // ~ins

        auto it = asmTable.find(token);
        if (it == asmTable.end()){
            ERROR("wrong asm command \"" << TERMCOLOR::FG_LBLUE << token << TERMCOLOR::LOG_DEFAULT << "\" on line " << line << '.');
        }
        lastInstruction = token;
        commandInfo = &(it->second);
        ctx.bcFile << (char)((*commandInfo)[0]);
        stepsToNext = commandInfo->size() - 1;
        token.clear();
        #if DEBUG
            if (ArgVars::debugMode){
                ctx.bcFile << ':';
            }
        #endif
        ctx.byte++;
    }
    else {
        // Если token == значение (не команда)

        // ins
        if (insA == -2){
            // Первый аргумент ins.

            // Считываем размер следующего аргумета в байтах.
            if (!jmpMark.empty()){
                ERROR("line " << line << ". Jmp mark cannot be a size of a value.");
            }
            bool error;
            {
                std::vector<uint8_t> seq;
                ParseValue(token, seq, error);
                if(seq.size() > 4){
                    ERROR("first argument is >4 Bytes on line " << line << '.');
                }
                insA = 0;
                for (int i = 0; i < seq.size(); i++){
                    *((uint8_t*)(&insA) + i) = seq[i];
                }
            }
            if (error){
                ERROR("wrong value format on line " << line << '.');
            }
            if (insA < 0){
                ERROR("first argument is <0 on line " << line << '.');
            }
            stepsToNext--;
            token.clear();
            goto tokenProcEnd;
        }
        if (insA >= 0){
            // Второй аргумент ins.

            bool error;
            std::vector<uint8_t> val;

            // jmp
            if (jmpMark.empty()){
                ParseValue(token, val, error);
                if (insA != 0 && val.size() > insA){
                    ERROR("line " << line << ": Too large value \"" << token << "\". " << val.size() << "/" << insA << " Bytes.")
                }
            }
            else if (insA == 4 || insA == 0){
                ctx.InsertJmpMark(jmpMark, ctx.byte);
                int32_t toWrite = REGMEMAMOUNT;
                ctx.bcFile.write((char*)&toWrite, 4);
                ctx.byte += 4;
                jmpMark = {};
                insA = -1;
                stepsToNext--;
                goto tokenProcEnd;
            }
            else {
                ERROR((insA < 4 ? "not enough" : "too many") << " bytes in the argument to store a jmp mark " << TERMCOLOR::FG_LBLUE << ':' << jmpMark << ':' << TERMCOLOR::LOG_DEFAULT << " on line " << line << '.');
            }
            // ~jmp

            if (error){
                ERROR("wrong value \"" << TERMCOLOR::FG_LBLUE << token << TERMCOLOR::LOG_DEFAULT << "\" on line " << line << '.');
            }
            if (insA == 0){
                insA = val.size();
            }
            ctx.bcFile.write((const char*)&val[0], std::min(insA, (int)val.size()));
            for (int i = val.size(); i < insA; i++){
                ctx.bcFile << (uint8_t)0;
            }
            ctx.byte += insA;
            insA = -1;
            stepsToNext--;
            token.clear();
            goto tokenProcEnd;
        }
        // ~ins
        
        int argByteLen = (*commandInfo)[commandInfo->size() - stepsToNext];
        // TODO!!!! Убрать, когда закончу реализацию.
        if (argByteLen > 4){
            // TODO ?
            ERROR("values >4 Bytes are not suported yet. Line " << line << '.');
        }
        // jmp
        if (!jmpMark.empty()){
            if (argByteLen != 4){
                ERROR("not enough bytes in the argument to store a jmp mark " << TERMCOLOR::FG_LBLUE << ':' << jmpMark << ':' << TERMCOLOR::LOG_DEFAULT << " on line " << line << '.');
            }
            ctx.InsertJmpMark(jmpMark, ctx.byte);
            int32_t toWrite;
            if (lastInstruction == "jmpr"){
                toWrite = -ctx.byte + 1;
            }
            else {
                toWrite = REGMEMAMOUNT;
            }
            ctx.bcFile.write((char*)&toWrite, 4);
            ctx.byte += 4;
            stepsToNext--;
            jmpMark = {};
            goto tokenProcEnd;
        }
        // ~jmp

        std::vector<uint8_t> val;
        val.reserve(4);
        {
            auto it = regTable.find(token);
            if (it != regTable.end()){
                val.resize(4);
                *(int32_t*)(&val[0]) = it->second;
                goto regSkip1;
            }
        }
        {
            bool error;
            ParseValue(token, val, error);
            if (error || val.empty()){
                ERROR("wrong value \"" << TERMCOLOR::FG_LBLUE << token << TERMCOLOR::LOG_DEFAULT << "\" on line " << line << '.');
            }
        }
        if (val.size() > argByteLen){
            ERROR("line " << line << ": Too large value \"" << token << "\". " << val.size() << "/" << argByteLen << " Bytes.")
        }
    regSkip1:
        for (int i = 0; i < argByteLen; i++){
            ctx.bcFile << (i < val.size() ? val[i] : (uint8_t)0);
        }

        stepsToNext--;
        token.clear();
        // TODO? Удалить?
        #if DEBUG
            if (ArgVars::debugMode){
                if (stepsToNext == 0){
                    ctx.bcFile << '_';
                }else{
                    ctx.bcFile << '.';
                }
            }
        #endif
        ctx.byte += argByteLen;
    }

tokenProcEnd:
    if (!asmFile.eof()){
        goto cycle1;
    }
    // конец cycle1.
    // TODO мб заменить goto на while ? 
    
    if (stepsToNext != 0){
        ERROR("the last command do not have enough arguments");
    }

    
    return 0;
}


int AsmParserMode(){
    for(std::string& asmFileName : ArgVars::inpFiles){
        AsmParseContext ctx{};
        std::string bcFileName = asmFileName;
        {
            auto dot = bcFileName.find_last_of('.');
            if (dot != -1){
                bcFileName.resize(dot);
            }
            bcFileName += ".shbyte";
        }
        
        ctx.bcFile.open(bcFileName, ios::binary | ios::in | ios::out | ios::trunc);
        ctx.byte = 0;

        ParseAsm(asmFileName, ctx);
        // jpm
        for (auto& [jmpName, placesToInsert] : ctx.jmpList){
            if (placesToInsert[0] == -1){
                ERROR("Unknown jmpMark \"" << TERMCOLOR::FG_LBLUE << jmpName << TERMCOLOR::LOG_DEFAULT << "\".")
            }
            union{
                int32_t origin;
                uint8_t parts[4];
            };
            if (origin == -1){
                ERROR(TERMCOLOR::FG_LBLUE << ':' << jmpName << ':' << TERMCOLOR::LOG_DEFAULT << " is not defined.")
            }
            for (int j = 1; j < placesToInsert.size(); j++){
                ctx.bcFile.seekp(placesToInsert[j], ios::beg);
                ctx.bcFile >> parts[0] >> parts[1] >> parts[2] >> parts[3];
                origin += placesToInsert[0];
                ctx.bcFile.seekp(-4, ios::cur);
                ctx.bcFile << parts[0] << parts[1] << parts[2] << parts[3];
            }
        }

        // ~(jmp)
        LOG(" > " << TERMCOLOR::FG_LGREEN << "Done " << TERMCOLOR::LOG_DEFAULT << asmFileName);
        ctx.bcFile.close();
      ctx.jmpList.clear();
    }
    
    return 0;
}

void Init(){
    // DLog
    #if _DEBUG && DLOGISFILE
        __dLogFile.file.open(DLOGFILEPATH, ios::out | ios::trunc);
    #endif
}

// return 1 если jmpMark уже определена.
bool AsmParseContext::CreateJmpMark(const std::string_view& jmpMark, int bytePlaceInFile){
    auto it = jmpList.find(jmpMark);
    if (it == jmpList.end()){
        jmpList.insert({ (std::string)jmpMark, {bytePlaceInFile} });
    }
    else {
        if (it->second[0] != -1){
            return 1;
        }
        it->second[0] = bytePlaceInFile;
    }
    return 0;
}

void AsmParseContext::InsertJmpMark(const std::string_view& jmpMark, int bytePlaceInFile){
    auto it = jmpList.find(jmpMark);
    if (it == jmpList.end()){
        jmpList.insert({ (std::string)jmpMark, {-1, bytePlaceInFile} });
    }
    else{
        it->second.push_back(bytePlaceInFile);
    }
}       
