#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <stdint.h>
#include <vector>
#include <string>
#include "src/config.h"
#include "src/functions.h"
#include "src/types.h"
#include "src/tables.h"
#include "src/macros.h"
#include "src/macro_fns.h"
#include "src/modes.h"


namespace ArgVars {
    std::vector<fs::path> inpFiles;
    std::string outDir;
    bool printExitInfo = false;
    Modes mode;
}

namespace Memory {
    // TODO Посмотреть лимиты. Сейчас цифра взята с потолка.
    uint8_t memory[(int)4e6];
}

void Init();


int main(int argc, char** argv){
    using namespace Memory;
    Init();

    if (int error = ParseArgs(argc, argv); error){
        return 0;
    }

    std::fill(memory, memory + REGMEMAMOUNT, 0);
    REG_inn = REGMEMAMOUNT;
    REG_sptr = REGMEMAMOUNT;

    int error = 0;

    if (ArgVars::mode & Modes::ASM){
        if (ArgVars::outDir.empty()){
            ArgVars::outDir = fs::current_path().generic_string();
        }
        error = AsmParserMode();
        
    }
    if (error != 0){
        AppExit(error);
    }

    if (ArgVars::mode & Modes::BYTECODE)
        error = VMachineMode();
    AppExit(error);
}

void Init(){
    // DLog
    #if _DEBUG && DLOGISFILE
        __dLogFile.file.open(DLOGFILEPATH, ios::out | ios::trunc);
    #endif
}
