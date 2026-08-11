#pragma once

template<typename T>
class DefaultDeleter{

    void operator()(T* ptr){
        delete ptr;
    }

};
