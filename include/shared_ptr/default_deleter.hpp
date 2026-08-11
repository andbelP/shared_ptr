#pragma once

template<typename T>
class DefaultDeleter{
public:
    void operator()(T* ptr){
        delete ptr;
    }

};
