#pragma once


template<typename T>
WeakPtr<T>& WeakPtr<T>::operator=(const WeakPtr& other) noexcept {
    if(this == &other){
        return *this;
    }
    WeakPtr tmp{other};
    swap(tmp);
    return *this;
}

template<typename T>
template<typename U> requires std::is_convertible_v<U*, T*>
WeakPtr<T>& WeakPtr<T>::operator=(const WeakPtr<U>& other) noexcept {
    WeakPtr tmp{other};
    swap(tmp);
    return *this;
}

template<typename T>
template<typename U> requires std::is_convertible_v<U*, T*>
WeakPtr<T>& WeakPtr<T>::operator=(const SharedPtr<U>& other) noexcept {
    WeakPtr tmp(other);
    swap(tmp);
    return *this;
}

template<typename T>
template<typename U> requires std::is_convertible_v<U*, T*>
WeakPtr<T>& WeakPtr<T>::operator=(WeakPtr<U>&& other) noexcept {
    WeakPtr tmp{};
    swap(tmp);
    cblock_ = other.cblock_;
    data_=other.data_;
    other.data_=nullptr;
    other.cblock_ = nullptr;
    return *this;
}

template<typename T>
WeakPtr<T>& WeakPtr<T>::operator=(WeakPtr&& other) noexcept {
    if(this == &other){
        return *this;
    }
    WeakPtr tmp{};
    swap(tmp);
    cblock_ = other.cblock_;
    data_=other.data_;
    other.data_=nullptr;
    other.cblock_ = nullptr;
    return *this;
}