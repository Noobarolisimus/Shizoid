#include "macro_fns.h"

#if COLORTERMINAL

    #if _WIN && RELEASE
        StaticInit<HANDLE, WinColorTermHandleInit> __hConsole;
        HANDLE WinColorTermHandleInit::operator() (){
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            if (hConsole == INVALID_HANDLE_VALUE)
            {
                DLOG_STR("There no color terminal support :(" << std::endl);
                DLOG_STR("Color terminal error: " << GetLastError() << std::endl);
            }
            return hConsole;
        }
    #else
        TERMCOLOR::TERMCOLOR(const char* str, size_t size) : color(std::string(str, size)) {}
        TERMCOLOR::TERMCOLOR(std::string str) : color(str) {}

        TERMCOLOR operator| (const TERMCOLOR& lhs, const TERMCOLOR& rhs){
            return TERMCOLOR(lhs.color + rhs.color);
        }

        std::ostream& operator<< (std::ostream& stream, const TERMCOLOR& rhs){
            return stream << rhs.color;
        }

        std::ostream& operator<< (std::ostream& stream, TERMCOLOR& rhs){
            return stream << rhs.color;
        }

        TERMCOLOR operator"" _TC(const char* str, size_t size){
            return TERMCOLOR(str, size);
        }

        #pragma region TERMCOLOR consts
        const TERMCOLOR TERMCOLOR::FG_DEFAULT = "\x1b[39m"_TC;
        const TERMCOLOR TERMCOLOR::BG_DEFAULT = "\x1b[49m"_TC;
        const TERMCOLOR TERMCOLOR::DEFAULT    = TERMCOLOR::FG_DEFAULT | TERMCOLOR::BG_DEFAULT;
        const TERMCOLOR TERMCOLOR::FG_BLACK   = "\x1b[30m"_TC;
        const TERMCOLOR TERMCOLOR::FG_RED     = "\x1b[31m"_TC;
        const TERMCOLOR TERMCOLOR::FG_GREEN   = "\x1b[32m"_TC;
        const TERMCOLOR TERMCOLOR::FG_YELLOW  = "\x1b[33m"_TC;
        const TERMCOLOR TERMCOLOR::FG_BLUE    = "\x1b[34m"_TC;
        const TERMCOLOR TERMCOLOR::FG_MAGENTA = "\x1b[35m"_TC;
        const TERMCOLOR TERMCOLOR::FG_CYAN    = "\x1b[36m"_TC;
        const TERMCOLOR TERMCOLOR::FG_WHITE   = "\x1b[37m"_TC;
        const TERMCOLOR TERMCOLOR::FG_LBLACK   = TERMCOLOR::FG_BLACK;
        const TERMCOLOR TERMCOLOR::FG_LRED     = "\x1b[38;5;9m"_TC;//TERMCOLOR::FG_RED;
        const TERMCOLOR TERMCOLOR::FG_LGREEN   = TERMCOLOR::FG_GREEN;
        const TERMCOLOR TERMCOLOR::FG_LYELLOW  = "\x1b[38;5;11m"_TC;//TERMCOLOR::FG_YELLOW;
        const TERMCOLOR TERMCOLOR::FG_LBLUE    = "\x1b[38;5;6m"_TC;//TERMCOLOR::FG_BLUE;
        const TERMCOLOR TERMCOLOR::FG_LMAGENTA = TERMCOLOR::FG_MAGENTA;
        const TERMCOLOR TERMCOLOR::FG_LCYAN    = TERMCOLOR::FG_CYAN;
        const TERMCOLOR TERMCOLOR::FG_LWHITE   = TERMCOLOR::FG_WHITE;
        const TERMCOLOR TERMCOLOR::BG_BLACK   = "\x1b[40m"_TC;
        const TERMCOLOR TERMCOLOR::BG_RED     = "\x1b[41m"_TC;
        const TERMCOLOR TERMCOLOR::BG_GREEN   = "\x1b[42m"_TC;
        const TERMCOLOR TERMCOLOR::BG_YELLOW  = "\x1b[43m"_TC;
        const TERMCOLOR TERMCOLOR::BG_BLUE    = "\x1b[44m"_TC;
        const TERMCOLOR TERMCOLOR::BG_MAGENTA = "\x1b[45m"_TC;
        const TERMCOLOR TERMCOLOR::BG_CYAN    = "\x1b[46m"_TC;
        const TERMCOLOR TERMCOLOR::BG_WHITE   = "\x1b[47m"_TC;
        const TERMCOLOR TERMCOLOR::BG_LBLACK   = TERMCOLOR::BG_BLACK;
        const TERMCOLOR TERMCOLOR::BG_LRED     = TERMCOLOR::BG_RED;
        const TERMCOLOR TERMCOLOR::BG_LGREEN   = TERMCOLOR::BG_GREEN;
        const TERMCOLOR TERMCOLOR::BG_LYELLOW  = TERMCOLOR::BG_YELLOW;
        const TERMCOLOR TERMCOLOR::BG_LBLUE    = TERMCOLOR::BG_BLUE;
        const TERMCOLOR TERMCOLOR::BG_LMAGENTA = TERMCOLOR::BG_MAGENTA;
        const TERMCOLOR TERMCOLOR::BG_LCYAN    = TERMCOLOR::BG_CYAN;
        const TERMCOLOR TERMCOLOR::BG_LWHITE   = TERMCOLOR::BG_WHITE;
        const TERMCOLOR TERMCOLOR::LOG_DEFAULT   = TERMCOLOR::FG_LMAGENTA;
        #pragma endregion
    #endif
#else // !COLORTERMINAL

    TERMCOLOR operator| (const TERMCOLOR& lhs, const TERMCOLOR& rhs){
        return {};
    }

    std::ostream& operator<< (std::ostream& stream, const TERMCOLOR& rhs){
        return stream;
    }

    std::ostream& operator<< (std::ostream& stream, TERMCOLOR& rhs){
        return stream;
    }

    TERMCOLOR operator"" _TC(const char* str, size_t size){
        return TERMCOLOR();
    }

    #pragma region TERMCOLOR consts
    const TERMCOLOR TERMCOLOR::FG_DEFAULT = {};
    const TERMCOLOR TERMCOLOR::BG_DEFAULT = {};
    const TERMCOLOR TERMCOLOR::DEFAULT    = {};
    const TERMCOLOR TERMCOLOR::FG_BLACK   = {};
    const TERMCOLOR TERMCOLOR::FG_RED     = {};
    const TERMCOLOR TERMCOLOR::FG_GREEN   = {};
    const TERMCOLOR TERMCOLOR::FG_YELLOW  = {};
    const TERMCOLOR TERMCOLOR::FG_BLUE    = {};
    const TERMCOLOR TERMCOLOR::FG_MAGENTA = {};
    const TERMCOLOR TERMCOLOR::FG_CYAN    = {};
    const TERMCOLOR TERMCOLOR::FG_WHITE   = {};
    const TERMCOLOR TERMCOLOR::FG_LBLACK   = {};
    const TERMCOLOR TERMCOLOR::FG_LRED     = {};
    const TERMCOLOR TERMCOLOR::FG_LGREEN   = {};
    const TERMCOLOR TERMCOLOR::FG_LYELLOW  = {};
    const TERMCOLOR TERMCOLOR::FG_LBLUE    = {};
    const TERMCOLOR TERMCOLOR::FG_LMAGENTA = {};
    const TERMCOLOR TERMCOLOR::FG_LCYAN    = {};
    const TERMCOLOR TERMCOLOR::FG_LWHITE   = {};
    const TERMCOLOR TERMCOLOR::BG_BLACK   = {};
    const TERMCOLOR TERMCOLOR::BG_RED     = {};
    const TERMCOLOR TERMCOLOR::BG_GREEN   = {};
    const TERMCOLOR TERMCOLOR::BG_YELLOW  = {};
    const TERMCOLOR TERMCOLOR::BG_BLUE    = {};
    const TERMCOLOR TERMCOLOR::BG_MAGENTA = {};
    const TERMCOLOR TERMCOLOR::BG_CYAN    = {};
    const TERMCOLOR TERMCOLOR::BG_WHITE   = {};
    const TERMCOLOR TERMCOLOR::BG_LBLACK   = {};
    const TERMCOLOR TERMCOLOR::BG_LRED     = {};
    const TERMCOLOR TERMCOLOR::BG_LGREEN   = {};
    const TERMCOLOR TERMCOLOR::BG_LYELLOW  = {};
    const TERMCOLOR TERMCOLOR::BG_LBLUE    = {};
    const TERMCOLOR TERMCOLOR::BG_LMAGENTA = {};
    const TERMCOLOR TERMCOLOR::BG_LCYAN    = {};
    const TERMCOLOR TERMCOLOR::BG_LWHITE   = {};
    const TERMCOLOR TERMCOLOR::LOG_DEFAULT = {};
    #pragma endregion
#endif


// DLog
#if DEBUG
    #if DLOGISFILE
        __DLogFile::~__DLogFile(){
            file.close();
        }
        __DLogFile __dLogFile;
    #endif
#endif
