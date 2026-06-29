#pragma once

#include <exception>
#include <string>

#include "control_block.hpp"
#include "shared_ptr.hpp"

class bad_weak_ptr : public std::exception {
    std::string msg_;

   public:

    bad_weak_ptr(std::string_view data) : msg_(data){}

    const char* what() const noexcept override {
        return msg_.data();
    }
};

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
    weak_ptr(const shared_ptr<U>& ptr) {
        data_ = ptr.data_;
        cblock_ = ptr.cblock_;

        if (cblock_) {
            cblock_->IncreaseWeak();
        }
    }

    weak_ptr(const shared_ptr& ptr) {
        data_ = ptr.data_;
        cblock_ = ptr.cblock_;

        if (cblock_) {
            cblock_->IncreaseWeak();
        }
    }

    bool expired() { return !cblock_ || cblock_->strong_cnt > 0; }
};

template <typename T>
template <typename U>
shared_ptr<T>::shared_ptr(const weak_ptr<U>& other) {
    if (other.expired()) {
        throw bad_weak_ptr("Can't initialize shared_ptr with expired weak_ptr");
    }
    data_=other.data_;
    cblock_=other.cblock_;
    cblock_->IncreaseStrong();
};