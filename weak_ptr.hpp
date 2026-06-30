#pragma once

#include <exception>
#include <string>
#include <utility>

#include "control_block.hpp"
#include "shared_ptr.hpp"

class bad_weak_ptr : public std::exception {
    std::string msg_;

   public:
    bad_weak_ptr(std::string_view data);

    const char* what() const noexcept override;
};

inline bad_weak_ptr::bad_weak_ptr(std::string_view data) : msg_(data) {}

inline const char* bad_weak_ptr::what() const noexcept { return msg_.data(); }

template <typename T>
class weak_ptr {
    T* data_{};

    ControlBlock* cblock_{};

    template <typename U>
    friend class weak_ptr;

    template <typename U>
    friend class shared_ptr;

   public:
    template <typename U>
    weak_ptr(const shared_ptr<U>& ptr);

    weak_ptr(const shared_ptr<T>& ptr);

    weak_ptr() = default;

    template <typename U>
    weak_ptr(const weak_ptr<U>& ptr);

    weak_ptr(const weak_ptr& ptr);

    weak_ptr& operator=(const weak_ptr& other);

    template <typename U>
    weak_ptr& operator=(const weak_ptr<U>& other);

    void swap(weak_ptr& other) noexcept;

    void reset() noexcept;

    shared_ptr<T> lock();

    bool expired() const;

    long use_count() const noexcept;

};

template <typename T>
void weak_ptr<T>::swap(weak_ptr& other) noexcept {
    std::swap(data_, other.data_);
    std::swap(cblock_, other.cblock_);
}

template <typename T>
template <typename U>
weak_ptr<T>::weak_ptr(const shared_ptr<U>& ptr) {
    data_ = ptr.data_;
    cblock_ = ptr.cblock_;

    if (cblock_) {
        cblock_->IncreaseWeak();
    }
}

template <typename T>
weak_ptr<T>::weak_ptr(const shared_ptr<T>& ptr) {
    data_ = ptr.data_;
    cblock_ = ptr.cblock_;

    if (cblock_) {
        cblock_->IncreaseWeak();
    }
}

template <typename T>
shared_ptr<T> weak_ptr<T>::lock() {
    if (expired()) {
        return shared_ptr<T>{};
    }
    return shared_ptr<T>(*this);
}

template <typename T>
bool weak_ptr<T>::expired() const {
    return !cblock_ || cblock_->strong_cnt > 0;
}

template <typename T>
template <typename U>
shared_ptr<T>::shared_ptr(const weak_ptr<U>& other) {
    if (other.expired()) {
        throw bad_weak_ptr("Can't initialize shared_ptr with expired weak_ptr");
    }
    data_ = other.data_;
    cblock_ = other.cblock_;
    cblock_->IncreaseStrong();
};

template <typename T>
weak_ptr<T>& weak_ptr<T>::operator=(const weak_ptr<T>& other) {
    weak_ptr tmp = other;
    swap(tmp);
    return *this;
}

template <typename T>
template <typename U>
weak_ptr<T>& weak_ptr<T>::operator=(const weak_ptr<U>& other) {
    weak_ptr tmp = other;
    swap(tmp);
    return *this;
}

template<typename T>
void weak_ptr<T>::reset() noexcept{
    weak_ptr<T>{}.swap(*this);
}

template<typename T>
long weak_ptr<T>::use_count() const noexcept{
    if(!cblock_){
        return 0;
    }
    return cblock_->strong_cnt;
}

template <typename T>
weak_ptr<T>::weak_ptr(const weak_ptr<T>& ptr) {
    data_ = ptr.data_;
    cblock_ = ptr.cblock_;

    if (cblock_) {
        cblock_->IncreaseWeak();
    }
}

template <typename T>
template <typename U>
weak_ptr<T>::weak_ptr(const weak_ptr<U>& ptr) {
    data_ = ptr.data_;
    cblock_ = ptr.cblock_;

    if (cblock_) {
        cblock_->IncreaseWeak();
    }
}