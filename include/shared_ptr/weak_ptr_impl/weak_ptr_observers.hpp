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

template<typename T>
long WeakPtr<T>::use_count() const noexcept{
    if(!cblock_){
        return 0;
    }
    return cblock_->GetStrongCounter();
}

template<typename T>
bool WeakPtr<T>::expired() const noexcept{
    return use_count() == 0;
}

template<typename T>
SharedPtr<T> WeakPtr<T>::lock() const noexcept {
    if(expired()){
        return SharedPtr<T>{};
    }
    return SharedPtr<T>{*this};
}