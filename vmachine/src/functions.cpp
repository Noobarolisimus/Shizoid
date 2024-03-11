#include "functions.h"
#include "macro_fns.h"


void AppExit(int code){
    SETCOLOR(TERMCOLOR::DEFAULT);
    exit(code);
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

    bool nextIsOFile = false;
    for(int i = 1; i < argc; i++){
        if (strncmp(argv[i], "--", 2) == 0){
            char* arg = argv[i] + 2;
            if (strcmp(arg, "asm") == 0){
                ArgVars::mode = Modes::ASM;
                continue;
            }
            if (strcmp(arg, "bytecode") == 0){
                ArgVars::mode = Modes::BYTECODE;
                continue;
            }
            if (strcmp(arg, "both") == 0){
                ArgVars::mode = Modes::BOTH;
                continue;
            }
            if (strcmp(arg, "exitinfo") == 0){
                printExitInfo = true;
                continue;
            }
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
            LOG("There is no \"" << argv[i] << "\" file or dir");
            return 1;
        }
        if (nextIsOFile){
            nextIsOFile = false;
            if(!fs::is_directory(argv[i])){
                LOG('"' << argv[i] << "\" is not a directory (-o)");
                return 1;
            }
            outDir = argv[i];
            if (outDir[outDir.size() - 1] != DIRDELIM){
                outDir += DIRDELIM;
            }
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

void GetOutPath(const fs::path& asmPath, fs::path& outBytecodePath){
    if (ArgVars::outDir.empty()){
        outBytecodePath = asmPath;
    }
    else {
        outBytecodePath = ArgVars::outDir;
        outBytecodePath += asmPath.stem();
    }
    outBytecodePath.replace_extension("shbyte");
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
    return character;
}


// void LongAdd(uint8_t a, std::vector<uint8_t>& b, int bytePlaceToAdd){
//     
// }

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
            // Возможно сложение заходит на память вне outBytes.
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
            // Возможно стоит находить все экранированые символы и учитывать это?
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
