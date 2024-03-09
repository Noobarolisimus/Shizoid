#include "types.h"

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
