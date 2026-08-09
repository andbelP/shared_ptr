#pragma once

#include "control_block.hpp"


template<typename T>
class SharedPtr {
    T* data_;
    ControlBlock* cblock_;

public:

    SharedPtr(T* data) : data_(data), cblock_(new TypedControlBlock<T>{1,0}) {}

}