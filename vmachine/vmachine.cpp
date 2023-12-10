#include <crtdbg.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "src/tables.h"

void Init();

// Посмотреть лимиты. Сейчас цифра взята с потолка
uint8_t memory[(int)4e6];

int main(int argc, char** argv){
    Init();
    if(argc == 1){
        std::cout << "Pass a shizoid bytecode file path (.shasm)\n";
        return 0;
    }
    if(argc > 2){
        std::cout << "Too many arguments. Pass a shizoid bytecode file path (.shasm)\n";
        return 0;
    }
    if(!std::filesystem::exists(argv[1])){
        std::cout << "There is no \"" << argv[1] << "\" file";
        return 0;
    }

    std::fill(memory, memory + REGMEMAMOUNT, 0);
    REG_inn = REGMEMAMOUNT;
    REG_sptr = REGMEMAMOUNT;

    

    // Читаем программу
    std::ifstream asmFile(argv[1]);
    while(!asmFile.eof()){
        asmFile.read((char*)memory + REG_sptr, 1);
        REG_sptr++;
    }

    

    
}

void Init(){
    
}