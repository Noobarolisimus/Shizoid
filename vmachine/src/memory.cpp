#include "memory.h"
#include <cstdint>
#include <optional>

namespace Memory {
    // TODO Посмотреть лимиты. Сейчас цифра взята с потолка.
    _Memory memory;
    
    // //  class Ptr .
    // Ptr::Ptr(const uint32_t& ptr){
    //     rawPtr = ptr;
    // }

    // Ptr& Ptr::operator+= (const Ptr& rhs){
    //     rawPtr += rhs.rawPtr;
    // }

    // Ptr& Ptr::operator-= (const Ptr& rhs){
    //     rawPtr -= rhs.rawPtr;
    // }

    // Ptr operator+ (const Ptr& a, const Ptr& b){
    //     return Ptr(a.rawPtr + b.rawPtr);
    // }

    // Ptr operator- (const Ptr& a, const Ptr& b){
    //     return Ptr(a.rawPtr - b.rawPtr);
    // }

    // //  class Ptr .
    
    // ~class _Memory .
    _Memory::_Memory() {
        CreatePage();
    }
    
    std::optional<Ptr> _Memory::CreatePage() {
        pages[sp] = malloc(SPACE_SIZE);
        if (pages[sp] == 0){
            return std::nullopt;
        }
        if (sp == 0){
            base = pages[0];
        }
        return sp++ << PAGE_BIT_OFFSET;
    }
    
    void _Memory::DeletePage() {
        sp--;
        free(pages[sp]);
        if (sp == 0){
            base = nullptr;
        }
    }
    
    // Начало памяти.
    uint8_t* _Memory::Base() {
        return (uint8_t*)base;
    }
    
    int32_t& _Memory::operator[] (const Ptr& ptr) {
        return *(int32_t*)((uint8_t*)pages[ptr >> PAGE_BIT_OFFSET] + (ptr & SPACE_MASK));
    }
    
    int32_t& _Memory::LazyGet (const Ptr& ptr) {
        while ((ptr >> PAGE_BIT_OFFSET) >= sp) {
            CreatePage();
        }
        return *(int32_t*)((uint8_t*)pages[ptr >> PAGE_BIT_OFFSET] + (ptr & SPACE_MASK));
    }
    // ~class _Memory .
} // namespace Memory.

