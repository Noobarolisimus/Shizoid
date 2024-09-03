#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <stdint.h>
#include <vector>
#include <string>
#include "src/config.h"
#include "memory.h"
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


void Init();


int main(int argc, char** argv){
    using namespace Memory;
    Init();

    if (int error = ParseArgs(argc, argv); error){
        return 0;
    }

    std::fill(memory.Base(), memory.Base() + REGMEMAMOUNT, 0);
    REG_inn = 0;
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
