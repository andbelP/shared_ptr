#pragma once


template<typename T>
template<typename U> requires std::is_convertible_v<U*, T*>
WeakPtr<T>::WeakPtr(const SharedPtr<U>& ptr) noexcept : cblock_(ptr.cblock_), data_(ptr.data_) {
    if(cblock_){
        cblock_->IncreaseWeak();
    }
}

template<typename T>
WeakPtr<T>::WeakPtr(const WeakPtr& other) noexcept : cblock_(other.cblock_), data_(other.data_) {
    if(cblock_){
        cblock_->IncreaseWeak();
    }
}

template<typename T>
template<typename U> requires std::is_convertible_v<U*, T*>
WeakPtr<T>::WeakPtr(const WeakPtr<U>& other) noexcept : cblock_(other.cblock_), data_(other.data_) {
    if(cblock_){
        cblock_->IncreaseWeak();
    }
}

template<typename T>
WeakPtr<T>::WeakPtr(WeakPtr&& other) noexcept {
    cblock_ = other.cblock_;
    data_ = other.data_;
    other.cblock_ = nullptr;
    other.data_ = nullptr;
}

template<typename T>
template<typename U> requires std::is_convertible_v<U*, T*>
WeakPtr<T>::WeakPtr(WeakPtr<U>&& other) noexcept {
    cblock_ = other.cblock_;
    data_ = other.data_;
    other.cblock_ = nullptr;
    other.data_ = nullptr;
}

template<typename T>
WeakPtr<T>::~WeakPtr() {
    if(cblock_){
        cblock_->DecreaseWeak();
    }
}