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
    extern bool printExitInfo;
    extern Modes mode;
}

namespace Memory {
    // TODO Посмотреть лимиты. Сейчас цифра взята с потолка.
    extern uint8_t memory[];
}