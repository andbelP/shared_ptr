#pragma once


template<typename T>
void WeakPtr<T>::swap(WeakPtr& other) noexcept {
    std::swap(cblock_, other.cblock_);
    std::swap(data_, other.data_);
}

template<typename T>
void WeakPtr<T>::reset() noexcept {
    WeakPtr tmp;
    swap(tmp);
}