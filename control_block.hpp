#pragma once

#include <cstdint>

struct ControlBlock {
    std::size_t strong_cnt{};
    std::size_t weak_cnt{};

    ControlBlock(std::size_t sh_cnt, std::size_t wk_cnt)
        : strong_cnt(sh_cnt), weak_cnt(wk_cnt) {}

    void IncreaseStrong() { strong_cnt++; }
    void IncreaseWeak() { weak_cnt++; }

    void DecreaseStrong() {
        strong_cnt--;
        if (strong_cnt == 0) {
            DestroyObject();
        }
        if(strong_cnt==0 && weak_cnt==0){
            delete this;
        }
    }
    void DecreaseWeak() {
        weak_cnt--;
        if(strong_cnt==0 && weak_cnt==0){
            delete this;
        }
    }

    virtual void DestroyObject(){};

    virtual ~ControlBlock() = default;
};

template <typename T>
class TypedControlBlock : public ControlBlock {
    T* data_;

   public:
    TypedControlBlock(std::size_t sh_cnt, std::size_t wk_cnt, T* data)
        : ControlBlock(sh_cnt, wk_cnt), data_(data) {}

    void DestroyObject() override {
        delete data_;
    };
};