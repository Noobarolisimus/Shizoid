#pragma once
#include <cstdint>
#include <optional>


namespace Memory {
    constexpr uint32_t PAGE_MASK = 0xff00'0000;
    constexpr uint32_t SPACE_MASK = 0x00ff'ffff;
    using Ptr = uint32_t;
    // class Ptr {
    //     uint32_t rawPtr = 0;
        
    //     Ptr() = default;
    //     Ptr(const Ptr&) = default;
    //     Ptr(const uint32_t& rawPtr);
    //     Ptr& operator+= (const Ptr& rhs);
    //     Ptr& operator-= (const Ptr& rhs);
    //     friend Ptr operator+ (const Ptr& a, const Ptr& b);
    //     friend Ptr operator- (const Ptr& a, const Ptr& b);
    // };

    // Сейчас страницы памяти растётут/удаляются как в стаке.
    // Мб сделаю динамическое выделение.
    class _Memory {
    private:
        void* pages[256];
        int sp = 0; // stack pointer.
        void* base;
    public:
        static const uint32_t SPACE_SIZE = 0x1000000; // 2**24, 16 МБ, размер space'а.
        static const int PAGE_BIT_OFFSET = 24;
        _Memory();
        // Возвращает `Ptr` на первый элемент страницы.
        std::optional<Ptr> CreatePage();
        void DeletePage();
        uint8_t* Base();
        int32_t& operator[] (const Ptr& ptr);
        int32_t& LazyGet(const Ptr& ptr);
    };

    extern _Memory memory;
}
