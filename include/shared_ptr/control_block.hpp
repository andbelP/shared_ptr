#pragma once

#include <cassert>

class ControlBlock {
    std::size_t strong_cnt_{};
    std::size_t weak_cnt_{};

protected:

    ControlBlock(std::size_t strong_cnt, std::size_t weak_cnt) : strong_cnt_(strong_cnt), weak_cnt_(weak_cnt) {}

public:

    virtual void DestroyObject() = 0;

    virtual ~ControlBlock() = default;

    void IncreaseStrong() { ++strong_cnt_; }

    void IncreaseWeak() { ++weak_cnt_; }

    void DecreaseWeak() {
        assert(weak_cnt_ > 0);

        --weak_cnt_;
        if(weak_cnt_ == 0 && strong_cnt_ == 0){
            delete this;
        }
    }

    void DecreaseStrong() {
        assert(strong_cnt_ > 0);

        --strong_cnt_;
        if(strong_cnt_ == 0 && weak_cnt_ == 0){
            DestroyObject();
            delete this;
        }
        else if(strong_cnt_ == 0){
            DestroyObject();
        }
    }

};

template<typename T>
class TypedControlBlock : public ControlBlock{

    T* data_;

public:

    TypedControlBlock(std::size_t weak_cnt, std::size_t strong_cnt, T* data) : ControlBlock(weak_cnt, strong_cnt), data_(data) {}

    void DestroyObject() override {
        delete data_;
    }

};