#pragma once
#include <fstream>
#include <iostream>
#include "macros.h"

template<typename T>
struct __InitClassF{
    T operator() (){
        return T{};
    }
};

template <typename T, typename InitClassF = __InitClassF<T>>
class StaticInit{
public:
    T& Get(){
        static T o = InitClassF()();
        return o;
    }
};

// COLOR TERMINAL
#if _WIN && !DEBUG
    enum TERMCOLOR{
        BG_BLACK = 0, FG_BLACK = 0,
        BG_BLUE = 0x10, FG_BLUE = 0x1,
        BG_GREEN = 0x20, FG_GREEN = 0x2,
        BG_CYAN = 0x30, FG_CYAN = 0x3,
        BG_RED = 0x40, FG_RED = 0x4,
        BG_MAGENTA = 0x50, FG_MAGENTA = 0x5,
        BG_YELLOW = 0x60, FG_YELLOW = 0x6,
        BG_WHITE = 0x70, FG_WHITE = 0x7,
        BG_GRAY = 0x80, FG_GRAY = 0x8,
        BG_LCYAN = 0x90, FG_LCYAN = 0x9,
        BG_LGREEN = 0xA0, FG_LGREEN = 0xA,
        BG_LBLUE = 0xB0, FG_LBLUE = 0xB,
        BG_LRED = 0xC0, FG_LRED = 0xC,
        BG_LMAGENTA = 0xD0, FG_LMAGENTA = 0xD,
        BG_LYELLOW = 0xE0, FG_LYELLOW = 0xE,
        BG_LWHITE = 0xF0, FG_LWHITE = 0xF,
        BG_DEFAULT = BG_BLACK, FG_DEFAULT = FG_WHITE,
        DEFAULT = BG_DEFAULT | FG_DEFAULT
    };
#else
    class TERMCOLOR{
        std::string color;
    public:
        TERMCOLOR() = default;
        TERMCOLOR(const char* str, size_t size);
        TERMCOLOR(std::string str);

        friend TERMCOLOR operator| (const TERMCOLOR& lhs, const TERMCOLOR& rhs);

        friend std::ostream& operator<< (std::ostream& stream, const TERMCOLOR& rhs);

        friend std::ostream& operator<< (std::ostream& stream, TERMCOLOR& rhs);

        #pragma region consts

        static const TERMCOLOR FG_DEFAULT;
        static const TERMCOLOR BG_DEFAULT;
        static const TERMCOLOR    DEFAULT;
        
        static const TERMCOLOR FG_BLACK  ;
        static const TERMCOLOR FG_RED    ;
        static const TERMCOLOR FG_GREEN  ;
        static const TERMCOLOR FG_YELLOW ;
        static const TERMCOLOR FG_BLUE   ;
        static const TERMCOLOR FG_MAGENTA;
        static const TERMCOLOR FG_CYAN   ;
        static const TERMCOLOR FG_WHITE  ;

        static const TERMCOLOR FG_LBLACK  ;
        static const TERMCOLOR FG_LRED    ;
        static const TERMCOLOR FG_LGREEN  ;
        static const TERMCOLOR FG_LYELLOW ;
        static const TERMCOLOR FG_LBLUE   ;
        static const TERMCOLOR FG_LMAGENTA;
        static const TERMCOLOR FG_LCYAN   ;
        static const TERMCOLOR FG_LWHITE  ;
        

        static const TERMCOLOR BG_BLACK  ;
        static const TERMCOLOR BG_RED    ;
        static const TERMCOLOR BG_GREEN  ;
        static const TERMCOLOR BG_YELLOW ;
        static const TERMCOLOR BG_BLUE   ;
        static const TERMCOLOR BG_MAGENTA;
        static const TERMCOLOR BG_CYAN   ;
        static const TERMCOLOR BG_WHITE  ;

        static const TERMCOLOR BG_LBLACK  ;
        static const TERMCOLOR BG_LRED    ;
        static const TERMCOLOR BG_LGREEN  ;
        static const TERMCOLOR BG_LYELLOW ;
        static const TERMCOLOR BG_LBLUE   ;
        static const TERMCOLOR BG_LMAGENTA;
        static const TERMCOLOR BG_LCYAN   ;
        static const TERMCOLOR BG_LWHITE  ;

        static const TERMCOLOR LOG_DEFAULT;

        #pragma endregion
    };

    TERMCOLOR operator"" __TC(const char* str, size_t size);
#endif
#if COLORTERMINAL
    #if _WIN && RELEASE
        #include <Windows.h>
        struct WinColorTermHandleInit{
            HANDLE operator() ();
        };
        extern StaticInit<HANDLE, WinColorTermHandleInit> __hConsole;
        #define SETCOLOR(color) SetConsoleTextAttribute(__hConsole.Get(), color);
    #else
        // Для линукса нет светлых цветов
        // UPD1: пиздёж сверху, https://habr.com/ru/company/macloud/blog/558316/


        #define SETCOLOR(color) std::cout << color;
    #endif
#else
    #define SETCOLOR(color)
#endif

// DLog
#if DEBUG
    #if DLOGISFILE
        class __DLogFile{
        public:
            std::fstream file;
            ~__DLogFile();
        };
        extern __DLogFile __dLogFile;
        // undef, чтобы не было варнинга
        #undef DLOGSTREAM
        #define DLOGSTREAM __dLogFile.file
    #endif

    // TODO: переделать с "ln"
    #define DLOG(info) {\
        SETCOLOR(TERMCOLOR::FG_LRED)\
        DLOGSTREAM << "DLOG: ";\
        SETCOLOR(TERMCOLOR::FG_BLUE)\
        DLOGSTREAM << #info << ": ";\
        SETCOLOR(TERMCOLOR::FG_LBLUE)\
        DLOGSTREAM << info << std::endl;\
        SETCOLOR(TERMCOLOR::FG_WHITE) }
    #define DLOG_TEXT(text) {\
        SETCOLOR(TERMCOLOR::FG_LRED)\
        DLOGSTREAM << "DLOG: ";\
        SETCOLOR(TERMCOLOR::FG_LBLUE)\
        DLOGSTREAM << text << std::endl;\
        SETCOLOR(TERMCOLOR::FG_WHITE) }
    #define DLOG_STR(str) {\
        SETCOLOR(TERMCOLOR::FG_LBLUE)\
        DLOGSTREAM << str;\
        SETCOLOR(TERMCOLOR::FG_WHITE) }
#else
    #define DLOG(info)
    #define DLOG_TEXT(text)
    #define DLOG_STR(str)
#endif

// LOG
#define LOG(info){\
    SETCOLOR(TERMCOLOR::LOG_DEFAULT)\
    std::cout << info << std::endl;\
    SETCOLOR(TERMCOLOR::FG_DEFAULT)}

#define LOG_STR(info){\
    SETCOLOR(TERMCOLOR::LOG_DEFAULT)\
    std::cout << info;\
    SETCOLOR(TERMCOLOR::FG_DEFAULT)}

// ERROR
#define ERROR(msg) {\
    SETCOLOR(TERMCOLOR::FG_RED)\
    std::cout << "ERROR: ";\
    SETCOLOR(TERMCOLOR::LOG_DEFAULT)\
    std::cout << msg << std::endl;\
    SETCOLOR(TERMCOLOR::FG_DEFAULT)}
    
// WARNING
#define WARNING(msg) {\
    SETCOLOR(TERMCOLOR::FG_LYELLOW)\
    std::cout << "Warning: ";\
    SETCOLOR(TERMCOLOR::LOG_DEFAULT)\
    std::cout << msg << std::endl;\
    SETCOLOR(TERMCOLOR::FG_DEFAULT)}

// DONLY
#if DEBUG
    #define DONLY(statment) statment
#else
    #define DONLY(statement)
#endif

// WINONLY & LINUXONLY
#if _WIN
    #define WINONLY(statment) statment
    #define LINUXONLY(statment)
#else
    #define WINONLY(statement)
    #define LINUXONLY(statment) statment
#endif


// ITTOP - iterator to pointer
#if _WIN
    #define ITTOP(iterator) iterator._Ptr
#else
    #define ITTOP(iterator) iterator.base()
#endif