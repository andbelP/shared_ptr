#pragma once

#include "control_block.hpp"
#include <utility>

template <typename T>
class shared_ptr {
    T* data_{};
    ControlBlock* cblock_{};

   public:

    void swap(shared_ptr& other){
        std::swap(data_, other.data_);
        std::swap(cblock_, other.cblock_);
    }

    shared_ptr() = default;

    shared_ptr(T* data){
        if(!data){
            return;
        }

        data_=data;
        cblock_=new TypedControlBlock<T>{1,0,data_};
    }

    shared_ptr(std::nullptr_t){}

    shared_ptr(const shared_ptr& ptr){
        data_=ptr.data_;
        cblock_=ptr.cblock_;

        if(cblock_){
            cblock_->IncreaseStrong();
        }
    }

    shared_ptr& operator=(const shared_ptr& ptr){
        shared_ptr tmp = ptr;
        swap(tmp);
        return *this;
    }

    ~shared_ptr(){
        if(cblock_){
            cblock_->DecreaseStrong();
        }
    }

};