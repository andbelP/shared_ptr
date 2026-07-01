#pragma once

#include <utility>

#include "control_block.hpp"

template <typename T>
class weak_ptr;

template <typename T>
class shared_ptr {
    T* data_{};
    ControlBlock* cblock_{};

    template <typename U>
    friend class weak_ptr;

    template <typename U>
    friend class shared_ptr;

   public:
    void swap(shared_ptr& other) noexcept;

    shared_ptr() = default;

    template <typename U>
    shared_ptr(U* data);

    shared_ptr(std::nullptr_t);

    shared_ptr(const shared_ptr& ptr);

    template <typename U>
    shared_ptr(const weak_ptr<U>& ptr);

    template <typename U>
    shared_ptr(const shared_ptr<U>& ptr);

    shared_ptr& operator=(const shared_ptr& ptr);

    T& operator*() const noexcept;

    T* operator->() const noexcept;

    T* get() const noexcept;

    template <typename U>
    shared_ptr& operator=(const shared_ptr<U>& ptr);

    long use_count() const noexcept;

    void reset() noexcept;

    template<typename U>
    void reset(U* ptr);

    ~shared_ptr();
};

template <typename T>
void shared_ptr<T>::swap(shared_ptr& other) noexcept {
    std::swap(data_, other.data_);
    std::swap(cblock_, other.cblock_);
}

template <typename T>
template <typename U>
shared_ptr<T>::shared_ptr(U* data) {
    if (!data) {
        return;
    }

    data_ = data;
    cblock_ = new TypedControlBlock<U>{1, 0, data};
}

template <typename T>
shared_ptr<T>::shared_ptr(std::nullptr_t) {}

template <typename T>
shared_ptr<T>::shared_ptr(const shared_ptr<T>& ptr) {
    data_ = ptr.data_;
    cblock_ = ptr.cblock_;

    if (cblock_) {
        cblock_->IncreaseStrong();
    }
}

template <typename T>
template <typename U>
shared_ptr<T>::shared_ptr(const shared_ptr<U>& ptr) {
    data_ = ptr.data_;
    cblock_ = ptr.cblock_;

    if (cblock_) {
        cblock_->IncreaseStrong();
    }
}

template <typename T>
shared_ptr<T>& shared_ptr<T>::operator=(const shared_ptr<T>& ptr) {
    shared_ptr tmp = ptr;
    swap(tmp);
    return *this;
}

template <typename T>
template <typename U>
shared_ptr<T>& shared_ptr<T>::operator=(const shared_ptr<U>& ptr) {
    shared_ptr tmp = ptr;
    swap(tmp);
    return *this;
}

template <typename T>
shared_ptr<T>::~shared_ptr() {
    if (cblock_) {
        cblock_->DecreaseStrong();
    }
}

template<typename T>
long shared_ptr<T>::use_count() const noexcept{
    if(cblock_){
        return cblock_->strong_cnt;
    }
    return 0;
}

template<typename T>
void shared_ptr<T>::reset() noexcept{
    shared_ptr<T>{}.swap(*this);
}

template<typename T>
template<typename U>
void shared_ptr<T>::reset(U* ptr){
    shared_ptr<T> tmp(ptr);
    swap(tmp);
}

template<typename T>
T& shared_ptr<T>::operator*() const noexcept{
    return *data_;
}

template<typename T>
T* shared_ptr<T>::operator->() const noexcept{
    return data_;
}

template<typename T>
T* shared_ptr<T>::get() const noexcept{
    return data_;
}