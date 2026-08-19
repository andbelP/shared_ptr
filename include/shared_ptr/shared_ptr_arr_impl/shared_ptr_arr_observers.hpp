#pragma once

#include "../weak_ptr.hpp"

template<typename T>
T* SharedPtr<T[]>::get() const noexcept {
    return data_;
}

template<typename T>
long SharedPtr<T[]>::use_count() const noexcept {
    if(cblock_){
        return cblock_->GetStrongCounter();
    }
    return 0;
}

template<typename T>
template<typename U>
bool SharedPtr<T[]>::owner_before(const SharedPtr<U>& other) const noexcept {
    return std::less<ControlBlock*>{}(cblock_, other.cblock_);
}

template<typename T>
template<typename U>
bool SharedPtr<T[]>::owner_before(const WeakPtr<U>& other) const noexcept {
    return std::less<ControlBlock*>{}(cblock_, other.cblock_);
}

template<typename T>
struct std::hash<SharedPtr<T[]>>{
    std::size_t operator()(const SharedPtr<T[]>& ptr) const noexcept{
        return std::hash<typename SharedPtr<T[]>::element_type*>{}(ptr.get());
    }
};