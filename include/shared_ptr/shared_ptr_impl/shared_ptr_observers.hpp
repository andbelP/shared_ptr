#pragma once


template<typename T>
T* SharedPtr<T>::get() const noexcept {
    return data_;
}

template<typename T>
long SharedPtr<T>::use_count() const noexcept {
    if(cblock_){
        return cblock_->GetStrongCounter();
    }
    return 0;
}

template<typename T>
template<typename U>
bool SharedPtr<T>::owner_before(const SharedPtr<U>& other) const noexcept {
    return std::less<ControlBlock*>{}(cblock_, other.cblock_);
}

template<typename T, typename U>
SharedPtr<T> static_pointer_cast(const SharedPtr<U>& ptr) noexcept{
    return SharedPtr<T>(ptr, static_cast<SharedPtr<T>::element_type*>(ptr.get()));
}  

template<typename T, typename U>
SharedPtr<T> dynamic_pointer_cast(const SharedPtr<U>& ptr) noexcept{
    auto* casted_ptr = dynamic_cast<SharedPtr<T>::element_type*>(ptr.get());
    if(casted_ptr){
        return SharedPtr<T>(ptr, casted_ptr);
    }
    return SharedPtr<T>{};
}

template<typename T, typename U>
SharedPtr<T> reinterpret_pointer_cast(const SharedPtr<U>& ptr) noexcept{
    return SharedPtr<T>(ptr, reinterpret_cast<SharedPtr<T>::element_type*>(ptr.get()));
}  

template<typename T, typename U>
SharedPtr<T> const_pointer_cast(const SharedPtr<U>& ptr) noexcept{
    return SharedPtr<T>(ptr, const_cast<SharedPtr<T>::element_type*>(ptr.get()));
}  

template<class Deleter, class T >
Deleter* get_deleter( const SharedPtr<T>& p ) noexcept {
    if(p.cblock_ && typeid(Deleter) == p.cblock_->GetDeleterTypeInfo()){
        return reinterpret_cast<Deleter*>(p.cblock_->GetDeleter());
    }
    return nullptr;
}

template<typename T>
struct std::hash<SharedPtr<T>>{
    std::size_t operator()(const SharedPtr<T>& ptr) const noexcept{
        return std::hash<typename SharedPtr<T>::element_type*>{}(ptr.get());
    }
};