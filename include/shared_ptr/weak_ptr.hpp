#pragma once

#include "../control_block/control_block.hpp"

template<typename T>
class SharedPtr;


template<typename T>
class WeakPtr{
    ControlBlock* cblock_{};
    
public:

    constexpr WeakPtr() noexcept = default;

    template<typename U>
    WeakPtr(const SharedPtr<U>& ptr) : cblock_(other.cblock_){
        if(cblock_){
            cblock_->IncreaseWeak();
        }
    }

    WeakPtr(const WeakPtr& other) : cblock_(other.cblock_){
        if(cblock_){
            cblock_->IncreaseWeak();
        }
    }

    template<typename U>
    WeakPtr(const WeakPtr<U>& other) : cblock_(other.cblock_){
        if(cblock_){
            cblock_->IncreaseWeak();
        }
    }

    WeakPtr(WeakPtr&& other) noexcept{
        cblock_=other.cblock_;
        other.cblock_=nullptr;
    }

    template<typename U>
    WeakPtr(WeakPtr<U>&& other) noexcept{
        cblock_=other.cblock_;
        other.cblock_=nullptr;
    }

    ~WeakPtr(){
        if(cblock_){
            cblock_->DecreaseWeak();
        }
    }

};