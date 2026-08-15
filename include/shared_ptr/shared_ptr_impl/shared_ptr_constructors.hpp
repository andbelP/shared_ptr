#pragma once


template<typename T>
SharedPtr<T>::SharedPtr() noexcept = default;

template<typename T>
SharedPtr<T>::SharedPtr(std::nullptr_t) noexcept {}

template<typename T>
SharedPtr<T>::SharedPtr(const SharedPtr& other) : data_(other.data_), cblock_(other.cblock_) {
    if(!other.cblock_){
        return;
    }

    cblock_->IncreaseStrong();

    assert(cblock_->GetStrongCounter() >= 2);
}

template<typename T>
SharedPtr<T>::SharedPtr(SharedPtr&& other) noexcept {
    swap(other);
}

template<typename T>
template<typename U> requires std::is_convertible_v<U*, T*>
SharedPtr<T>::SharedPtr(const SharedPtr<U>& other) : data_(other.data_), cblock_(other.cblock_) {
    if(!other.cblock_){
        return;
    }

    cblock_->IncreaseStrong();

    assert(cblock_->GetStrongCounter() >= 2);
}

template<typename T>
template<typename U> requires std::is_convertible_v<U*, T*>
SharedPtr<T>::SharedPtr(SharedPtr<U>&& other) noexcept
    : data_(other.data_), cblock_(other.cblock_) {
    other.data_ = nullptr;
    other.cblock_ = nullptr;
}

template<typename T>
template<typename Y> requires std::is_convertible_v<Y*, T*>
SharedPtr<T>::SharedPtr(Y* data)
    : data_(data), cblock_(new TypedControlBlock{1, 0, data, std::default_delete<Y>{}}) {}

template<typename T>
template<typename Y, typename Deleter> requires std::is_convertible_v<Y*, T*>
SharedPtr<T>::SharedPtr(Y* data, Deleter dltr)
    : data_(data), cblock_(new TypedControlBlock{1, 0, data, std::move(dltr)}) {}

template<typename T>
template<typename Deleter>
SharedPtr<T>::SharedPtr(std::nullptr_t, Deleter dltr)
    : cblock_(new TypedControlBlock{1, 0, static_cast<T*>(nullptr), std::move(dltr)}) {}

template<typename T>
template<typename Y>
SharedPtr<T>::SharedPtr(const SharedPtr<Y>& r, element_type* ptr) noexcept
    : data_(ptr), cblock_(r.cblock_) {
    if(!cblock_){
        return;
    }

    cblock_->IncreaseStrong();
}

template<typename T>
SharedPtr<T>::~SharedPtr() {
    if(cblock_){
        cblock_->DecreaseStrong();
    }
}

template<typename T>
template<typename Y, typename Deleter> requires std::is_convertible_v<Y*, T*>
SharedPtr<T>::SharedPtr(std::unique_ptr<Y, Deleter>&& other){
    Y* ptr = other.get();
    cblock_ = new TypedControlBlock{1, 0, ptr, other.get_deleter()};
    data_ = ptr;
    other.release();
}