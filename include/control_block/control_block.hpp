#pragma once

#include <cassert>

class ControlBlock {
    std::size_t strong_cnt_{};
    std::size_t weak_cnt_{};

protected:

    ControlBlock(std::size_t strong_cnt, std::size_t weak_cnt) : strong_cnt_(strong_cnt), weak_cnt_(weak_cnt) {}

    virtual void DestroyObject() = 0;


public:

    virtual void* GetDeleter() noexcept = 0;

    virtual const std::type_info& GetDeleterTypeInfo() noexcept = 0;

    std::size_t GetStrongCounter() {
        return strong_cnt_;
    }

    std::size_t GetWeakCounter() {
        return weak_cnt_;
    }

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

    virtual void* GetStoredPointer() const noexcept = 0;

    virtual ~ControlBlock() = default;

};

template<typename T, typename Deleter>
class TypedControlBlock : public ControlBlock{

    T* data_;
    Deleter dltr_;

public:

    TypedControlBlock(std::size_t weak_cnt, std::size_t strong_cnt, T* data, Deleter dltr) : ControlBlock(weak_cnt, strong_cnt), data_(data), dltr_(std::move(dltr)) {}

    void* GetStoredPointer() const noexcept override {
        return reinterpret_cast<void*>(data_);
    }

    void DestroyObject() override {
        dltr_(data_);
    }

    void* GetDeleter() noexcept override {
        return &dltr_;
    }

    const std::type_info& GetDeleterTypeInfo() noexcept override {
        return typeid(Deleter);
    }

};