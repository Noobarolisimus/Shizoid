#include <iostream>
#include <filesystem>
#include "config.h"
#include "macro_fns.h"
#include "modes.h"
#include "functions.h"
#include "types.h"


int VMachineMode(){
    using namespace Memory;
    fs::path bcPath;
    if (ArgVars::mode == Modes::BOTH){
        GetOutPath(ArgVars::inpFiles[0], bcPath);
    }
    else {
        bcPath = ArgVars::inpFiles[0];
    }

    // Читаем программу.
    std::ifstream bcFile(bcPath);
    
    while (!bcFile.eof()){
        bcFile.read((char*)memory + REG_sptr, 1);
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

// 0 - успешный парсинг.
// 1 - отмена парсинга, файл встречен не впервый раз (#once).
int ParseAsm(const fs::path& asmPath, AsmParseContext& ctx, const uint depth = 1){
    {
        auto result = std::find(ctx.dontInclude.begin(), ctx.dontInclude.end(), asmPath);
        if (result != ctx.dontInclude.end()){
            return 1;
        }
    }
    // Медленно из-за постоянного переключения цвета ? 
    LOG_STR(" ");
    for (int i = 0; i < depth; i++){
        LOG_STR(">");
    }
    LOG(" " << TERMCOLOR::FG_CYAN << "Processing " << TERMCOLOR::LOG_DEFAULT << asmPath);
    std::ifstream asmFile(asmPath);
    // Читаем пробелы.
    asmFile.unsetf(ios::skipws);
    std::string token;
    std::string lastInstruction;
    const std::vector<uint8_t>* commandInfo;
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
            // Сделать поиск по директориям и т.п. (=сделать умным).
            // (sub) Искать от inp файла
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
                // Медленно из-за постоянного переключения цвета? 
                LOG_STR(" ");
                for (int i = -1; i < (int)depth; i++){
                    LOG_STR(">");
                }
                LOG(" " << TERMCOLOR::FG_LGREEN << "Done " << TERMCOLOR::LOG_DEFAULT << fileName);
            }
        }
        else if (strcmp(str, "once") == 0){
            ctx.dontInclude.push_back((std::string)asmPath);
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
    for(fs::path& asmFileName : ArgVars::inpFiles){
        AsmParseContext ctx{};
        fs::path bcFilePath;
        GetOutPath(asmFileName, bcFilePath);
        
        ctx.bcFile.open(bcFilePath, ios::binary | ios::in | ios::out | ios::trunc);
        ctx.byte = 0;

        ParseAsm(asmFileName, ctx);
        // jpm
        for (auto& [jmpName, placesToInsert] : ctx.jmpList){
            int32_t origin = placesToInsert[0];
            if (origin == -1){
                ERROR(TERMCOLOR::FG_LBLUE << ':' << jmpName << ':' << TERMCOLOR::LOG_DEFAULT << " is not defined.")
            }
            for (int j = 1; j < placesToInsert.size(); j++){
                int32_t offset;
                ctx.bcFile.seekp(placesToInsert[j], ios::beg);
                ctx.bcFile.read((char*)&offset, 4);
                offset += placesToInsert[0];
                ctx.bcFile.seekp(-4, ios::cur);
                ctx.bcFile.write((char*)&offset, 4);
            }
        }

        // ~(jmp)
        LOG(" > " << TERMCOLOR::FG_LGREEN << "Done " << TERMCOLOR::LOG_DEFAULT << asmFileName);
        ctx.bcFile.close();
      ctx.jmpList.clear();
    }
    
    return 0;
}
