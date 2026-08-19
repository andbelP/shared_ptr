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
std::add_lvalue_reference_t<T> SharedPtr<T>::operator*() const noexcept
    requires (!std::is_void_v<T>) {
    return *data_;
}

template<typename T>
T* SharedPtr<T>::operator->() const noexcept {
    return data_;
}

template<typename T>
SharedPtr<T>::operator bool() const noexcept{
    return static_cast<bool>(data_);
}

template<typename T, typename U>
bool operator==(const SharedPtr<T>& lhs, const SharedPtr<U>& rhs) noexcept {
    return lhs.get() == rhs.get();
}

template<typename T>
bool operator==(const SharedPtr<T>& lhs, std::nullptr_t) noexcept {
    return lhs.get() ==nullptr;
}

template<typename U>
bool operator==(std::nullptr_t, const SharedPtr<U>& rhs) noexcept {
    return nullptr == rhs.get();
}

template<typename T, typename U>
auto operator<=>(const SharedPtr<T>& lhs, const SharedPtr<U>& rhs) noexcept {
    return std::compare_three_way{}(lhs.get(), rhs.get());
}


template<typename T>
std::ostream& operator<<(std::ostream& stream, const SharedPtr<T>& ptr) noexcept {
    stream << ptr.get();
    return stream;
}