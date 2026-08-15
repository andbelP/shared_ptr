#pragma once

#include "../weak_ptr.hpp"

template<typename T>
template<typename U>
bool WeakPtr<T>::owner_before(const WeakPtr<U>& other) const noexcept {
    return std::less<ControlBlock*>{}(cblock_, other.cblock_);
}

template<typename T>
template<typename U>
bool WeakPtr<T>::owner_before(const SharedPtr<U>& other) const noexcept {
    return std::less<ControlBlock*>{}(cblock_, other.cblock_);
}