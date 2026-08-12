#pragma once


template<typename T>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr& other) {
    if(this == &other){
        return *this;
    }
    SharedPtr tmp(other);
    swap(tmp);
    return *this;
}

template<typename T>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr&& other) noexcept {
    SharedPtr tmp(std::move(other));
    swap(tmp);
    return *this;
}

template<typename T>
template<typename U> requires std::is_convertible_v<U*, T*>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr<U>& other) {
    SharedPtr tmp(other);
    swap(tmp);
    return *this;
}

template<typename T>
template<typename U> requires std::is_convertible_v<U*, T*>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr<U>&& other) noexcept {
    SharedPtr tmp(std::move(other));
    swap(tmp);
    return *this;
}

template<typename T>
T& SharedPtr<T>::operator*() const noexcept {
    return *data_;
}

template<typename T>
T* SharedPtr<T>::operator->() const noexcept {
    return data_;
}
