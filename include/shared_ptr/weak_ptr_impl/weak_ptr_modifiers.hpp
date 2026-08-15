#pragma once


template<typename T>
void WeakPtr<T>::swap(WeakPtr& other) noexcept {
    std::swap(cblock_, other.cblock_);
}