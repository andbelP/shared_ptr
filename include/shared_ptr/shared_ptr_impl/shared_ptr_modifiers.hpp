#pragma once


template<typename T>
void SharedPtr<T>::swap(SharedPtr& other) noexcept {
    std::swap(data_, other.data_);
    std::swap(cblock_, other.cblock_);
}

template<typename T>
void SharedPtr<T>::reset() noexcept {
    SharedPtr tmp;
    swap(tmp);
}

template<typename T>
template<typename U> requires std::is_convertible_v<U*, T*>
void SharedPtr<T>::reset(U* ptr) {
    SharedPtr tmp(ptr);
    swap(tmp);
}

template<typename T>
template<typename U, typename Deleter> requires std::is_convertible_v<U*, T*>
void SharedPtr<T>::reset(U* ptr, Deleter dltr) {
    SharedPtr tmp(ptr, std::move(dltr));
    swap(tmp);
}
