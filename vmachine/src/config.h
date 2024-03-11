#pragma once
#include <filesystem>
#include <chrono>
#include <ranges>
#include "types.h"

namespace fs = std::filesystem;
namespace chrono = std::chrono;
namespace views = std::views;
using ios = std::ios;

namespace ArgVars {
    extern std::vector<fs::path> inpFiles;
    extern std::string outDir;
    extern Modes mode;
    extern bool printExitInfo;
}

namespace Memory {
    extern uint8_t memory[];
}