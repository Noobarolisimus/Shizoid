// lua generated
#include "tables.h"

char const HELPTEXT[] = "  VMachine.exe [--bytecode|--asm|--both] [--exitinfo] <files>... [-o <dir>]\n\n  --help\n  input file type:\n    --bytecode (default)\n      (.shbyte).\n      Virtual machine runs a program.\n      Only one file.\n      --exitinfo\n        Prints info about executing time and exit code after program finish.\n    --asm\n      (.shasm).\n      Creates bytecode files from shizoid assembler.\n      Any amount of files.\n      -o <dir>\n        Directory where .shbyte output files will be placed.\n        <dir> - current directory by default.\n    --both\n      Firstly runs --asm mode, than runs --bytecode with first input file.\n";

// { name, { bytecode, len A, len B, len ...} }
const std::map<std::string, std::vector<uint8_t>> asmTable{
    { "quit", { 0, 4 } },
    { "set", { 1, 4, 4 } },
    { "jmp", { 2, 4 } },
    { "copy", { 3, 4, 4 } },
    { "jifn", { 4, 4, 4 } },
    { "push", { 5, 4 } },
    { "pop", { 6, 4 } },
    { "fncr", { 7, 4 } },
    { "fnrt", { 8 } },
    { "nop", { 9 } },
    { "get", { 10, 4 } },
    { "putc", { 11, 1 } },
    { "putp", { 12, 4 } },
    { "ptov", { 13, 4, 4 } },
    { "pcp", { 14, 4, 4 } },
    { "jmpr", { 15, 4 } },
    { "add", { 65, 4, 4 } },
    { "sub", { 66, 4, 4 } },
    { "div", { 67, 4, 4 } },
    { "mod", { 68, 4, 4 } },
    { "mul", { 69, 4, 4 } },
    { "eql", { 70, 4, 4 } },
    { "less", { 71, 4, 4 } },
    { "lseq", { 72, 4, 4 } },
    { "and", { 73, 4, 4 } },
    { "or", { 74, 4, 4 } },
    { "not", { 75, 4 } },
    { "bxor", { 76, 4, 4 } },
    { "band", { 77, 4, 4 } },
    { "bor", { 78, 4, 4 } },
    { "bnot", { 79, 4 } },
    { "bshl", { 80, 4, 4 } },
    { "bshr", { 81, 4, 4 } },
    { "vmcall", { 82, 4 } },
};

const std::map<std::string, uint8_t> regTable{
    { "null", 0},
    { "inn", 8},
    { "sptr", 12},
    { "fndt", 16},
    { "ca", 20},
    { "cb", 21},
    { "cc", 22},
    { "cd", 23},
    { "ia", 24},
    { "ib", 28},
    { "ic", 32},
    { "id", 36},
};