// lua generated
#include "tables.h"

char const HELPTEXT[] = "  VMachine.exe [--bytecode|--asm|--both] <files...> [-o <dir>]\n\n  --help\n  input file type:\n    --bytecode (default)\n      (.shbyte)\n      Virtual machine runs a program.\n      Only one file.\n    --asm\n      (.shasm)\n      Creates bytecode files from shizoid assembler.\n      Any amount of files.\n      -o <dir>\n        Directory where .shbyte output files will be placed.\n        <dir> - current directory by default.\n      // --dupl\n      //   Create new files instead of replacing existing files in case of duplicates.\n      //   False by default\n    --both\n      Firstly runs --asm mode, than runs --bytecode with first input file.\n\n";

// { name, { bytecode, len A, len B, len ...} }
std::map<std::string, std::vector<uint8_t>> asmTable{
    { "nop", { 0 } },
    { "mov", { 1, 4, 4 } },
    { "jmp", { 2, 4 } },
    { "ptov", { 3, 4, 4 } },
    { "jifn", { 4, 4 } },
    { "spsh", { 5, 4 } },
    { "spop", { 6, 4 } },
    { "fncr", { 7, 4 } },
    { "fnrt", { 8 } },
    { "get", { 9, 4 } },
    { "putc", { 10, 1 } },
    { "putp", { 11, 4 } },
    { "quit", { 12, 4 } },
    { "add", { 65, 4, 4 } },
    { "sub", { 66, 4, 4 } },
    { "div", { 67, 4, 4 } },
    { "mod", { 68, 4, 4 } },
    { "mul", { 69, 4, 4 } },
    { "eql", { 70, 4, 4 } },
    { "less", { 71, 4, 4 } },
    { "and", { 72, 4, 4 } },
    { "or", { 73, 4, 4 } },
    { "not", { 74, 4 } },
    { "bxor", { 75, 4, 4 } },
    { "band", { 76, 4, 4 } },
    { "bor", { 77, 4, 4 } },
    { "bnot", { 78, 4 } },
    { "bshl", { 79, 4, 4 } },
    { "bshr", { 80, 4, 4 } },
};

std::map<std::string, uint8_t> regTable{
    { "null", 0},
    { "inn", 8},
    { "rinf", 12},
    { "sptr", 16},
    { "fndt", 20},
    { "fa", 24},
    { "fb", 32},
    { "fc", 40},
    { "ia", 48},
    { "ib", 52},
    { "ic", 56},
    { "ca", 60},
    { "cb", 61},
    { "cc", 62},
    { "cd", 63},
};