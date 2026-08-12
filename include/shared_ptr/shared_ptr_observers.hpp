#pragma once


template<typename T>
T* SharedPtr<T>::get() const noexcept {
    return data_;
}

template<typename T>
std::size_t SharedPtr<T>::use_count() const noexcept {
    if(cblock_){
        return cblock_->GetStrongCounter();
    }
    return 0;
}
