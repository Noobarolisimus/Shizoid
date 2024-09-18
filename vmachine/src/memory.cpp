#include "memory.h"
#include <cstdint>
#include <optional>

namespace mem {
    // TODO Посмотреть лимиты. Сейчас цифра взята с потолка.
    Memory memory;
    
    //  class Ptr .
    Memory::Ptr::Ptr(const uint32_t& ptr){
        innerPtr = ptr;
    }

    Memory::Ptr& Memory::Ptr::operator+= (const Memory::Ptr& rhs){
        innerPtr += rhs.innerPtr;
        return *this;
    }

    Memory::Ptr& Memory::Ptr::operator-= (const Memory::Ptr& rhs){
        innerPtr -= rhs.innerPtr;
        return *this;
    }

    Memory::Ptr operator+ (const Memory::Ptr& a, const Memory::Ptr& b){
        return Memory::Ptr(a.innerPtr + b.innerPtr);
    }

    Memory::Ptr operator- (const Memory::Ptr& a, const Memory::Ptr& b){
        return Memory::Ptr(a.innerPtr - b.innerPtr);
    }

    // ~class Ptr .
    
    // class _Memory .
    Memory::Memory() {
        CreatePage();
    }
    
    std::optional<Memory::Ptr> Memory::CreatePage() {
        pages[sp] = malloc(SPACE_SIZE); // TODO? calloc()?
        if (pages[sp] == 0){
            return std::nullopt;
        }
        if (sp == 0){
            base = pages[0];
        }
        return sp++ << PAGE_BIT_OFFSET;
    }
    
    void Memory::DeletePage() {
        sp--;
        free(pages[sp]);
        if (sp == 0){
            base = nullptr;
        }
    }
    
    int32_t* Memory::Base() {
        return (int32_t*)base;
    }
    
    int32_t& Memory::operator[] (const Ptr& ptr) {
        return *(int32_t*)((uint8_t*)pages[ptr.innerPtr >> PAGE_BIT_OFFSET] + (ptr.innerPtr & SPACE_MASK));
    }
    
    int32_t& Memory::LazyGet (const Ptr& ptr) {
        while ((ptr.innerPtr >> PAGE_BIT_OFFSET) >= sp) {
            CreatePage();
        }
        return *(int32_t*)((uint8_t*)pages[ptr.innerPtr >> PAGE_BIT_OFFSET] + (ptr.innerPtr & SPACE_MASK));
    }

    int32_t* Memory::ToCPtr(const Ptr& ptr){
        return &(*this)[ptr.innerPtr];
    }

    int32_t& Memory::ToValue(const Ptr& ptr){
        return (*this)[ptr.innerPtr];
    }

    // ~class _Memory .
} // namespace Memory.

