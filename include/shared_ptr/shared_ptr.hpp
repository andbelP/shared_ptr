#pragma once

#include "control_block.hpp"
#include "default_deleter.hpp"


template<typename T>
class SharedPtr {
    T* data_{};
    ControlBlock* cblock_{};

public:

    T* get() {
        return data_;
    }

    std::size_t use_count() const noexcept {
        if(cblock_){
            return cblock_->GetStrongCounter();
        }
        return 0;
    }

    T& operator*() const noexcept{
        return *data_;
    }

    T* operator->() const noexcept{
        return data_;
    }

    void swap(SharedPtr& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(cblock_, other.cblock_);
    }

    void reset() noexcept {
        SharedPtr tmp;
        swap(tmp);
    }

    template<typename U>
    void reset(U* ptr) noexcept {
        SharedPtr tmp(ptr);
        swap(tmp);
    }

    SharedPtr(const SharedPtr& other){
        if(!other.cblock_){
            return;
        }

        cblock_=other.cblock_;
        data_=other.data_;
        cblock_->IncreaseStrong();

        assert(cblock_->GetStrongCounter() >= 2);
    }

    SharedPtr& operator=(const SharedPtr& other){
        SharedPtr tmp(other);
        swap(tmp);
    }

    template<typename U>
    SharedPtr& operator=(const SharedPtr<U>& other){
        SharedPtr tmp(other);
        swap(tmp);
    }

    ~SharedPtr(){
        if(cblock_){
            cblock_->DecreaseStrong();
        }
    }

    template<typename Y>
    SharedPtr(Y* data) : data_(data), cblock_(new TypedControlBlock{1,0, data, DefaultDeleter<Y>{}}) {}

    template<typename Y, typename Deleter>
    SharedPtr(Y* data, Deleter dltr) : data_(data), cblock_(new TypedControlBlock{1,0, data, dltr}) {}

    SharedPtr(std::nullptr_t) {}

    SharedPtr() {}

};