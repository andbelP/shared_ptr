#pragma once

#include <type_traits>

#include "control_block.hpp"
#include "default_deleter.hpp"


template<typename T>
class SharedPtr {
    T* data_{};
    ControlBlock* cblock_{};

    template<typename U>
    friend class SharedPtr;

public:

    using element_type = std::remove_extent_t<T>;

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

    template<typename U, typename Deleter>
    void reset(U* ptr, Deleter dltr) noexcept {
        SharedPtr tmp(ptr, dltr);
        swap(tmp);
    }

    SharedPtr(const SharedPtr& other) : data_(other.data_), cblock_(other.cblock_) {
        if(!other.cblock_){
            return;
        }

        cblock_->IncreaseStrong();

        assert(cblock_->GetStrongCounter() >= 2);
    }

    template<typename U>
    SharedPtr(const SharedPtr<U>& other) : data_(other.data_), cblock_(other.cblock_) {
        if(!other.cblock_){
            return;
        }

        cblock_->IncreaseStrong();

        assert(cblock_->GetStrongCounter() >= 2);
    }

    SharedPtr& operator=(const SharedPtr& other){
        if(this == &other){
            return *this;
        }
        SharedPtr tmp(other);
        swap(tmp);
        return *this;
    }

    template<typename U>
    SharedPtr& operator=(const SharedPtr<U>& other){
        if(this == &other){
            return *this;
        }
        SharedPtr tmp(other);
        swap(tmp);
        return *this;
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
    
    template<typename Deleter>
    SharedPtr(std::nullptr_t, Deleter dltr) : cblock_(new TypedControlBlock{1,0, static_cast<T*>(nullptr), dltr}) {}

    template<typename Y> 
    SharedPtr(const SharedPtr<Y>& r, element_type* ptr) noexcept : data_(ptr), cblock_(r.cblock_) {
        if(!cblock_){
            return;
        }

        cblock_->IncreaseStrong();
    }

    SharedPtr() {}

};