#pragma once


template<typename T>
class SharedPtr<T[]> {
    T* data_{};
    ControlBlock* cblock_{};

    template<typename U>
    friend class SharedPtr;

    template<typename U>
    friend class WeakPtr;

    template<class Deleter, class U >
    friend Deleter* get_deleter( const SharedPtr<U>& p ) noexcept;

public:

    using element_type = T;

    using weak_type = WeakPtr<T[]>;

    SharedPtr() noexcept;
    SharedPtr(std::nullptr_t) noexcept;
    SharedPtr(const SharedPtr& other);
    SharedPtr(SharedPtr&& other) noexcept;

    template<typename U> requires std::is_convertible_v<U(*)[], T(*)[]>
    SharedPtr(const SharedPtr<U[]>& other);

    template<typename U> requires std::is_convertible_v<U(*)[], T(*)[]>
    SharedPtr(SharedPtr<U[]>&& other) noexcept;

    template<typename Y> requires std::is_convertible_v<Y*, T*>
    explicit SharedPtr(Y* data);

    template<typename Y, typename Deleter> requires std::is_convertible_v<Y*, T*>
    SharedPtr(Y* data, Deleter dltr);

    template<typename Deleter>
    SharedPtr(std::nullptr_t, Deleter dltr);

    template<typename Y>
    SharedPtr(const SharedPtr<Y>& r, element_type* ptr) noexcept;

    template<typename Y>
    SharedPtr(SharedPtr<Y>&& r, element_type* ptr) noexcept;

    template<typename Y, typename Deleter> requires std::is_convertible_v<Y*, T*>
    SharedPtr(std::unique_ptr<Y[], Deleter>&& ptr);

    template<typename Y> requires std::is_convertible_v<Y*, T*>
    explicit SharedPtr(const WeakPtr<Y[]>& r);

    ~SharedPtr();

    SharedPtr& operator=(const SharedPtr& other);
    SharedPtr& operator=(SharedPtr&& other) noexcept;

    template<typename U> requires std::is_convertible_v<U(*)[], T(*)[]>
    SharedPtr& operator=(const SharedPtr<U[]>& other);

    template<typename U> requires std::is_convertible_v<U(*)[], T(*)[]>
    SharedPtr& operator=(SharedPtr<U[]>&& other) noexcept;

    T& operator[](std::size_t index) const noexcept;

    explicit operator bool() const noexcept;

    void swap(SharedPtr& other) noexcept;
    void reset() noexcept;

    template<typename U> requires std::is_convertible_v<U(*)[], T(*)[]>
    void reset(U* ptr);

    template<typename U, typename Deleter> requires std::is_convertible_v<U(*)[], T(*)[]>
    void reset(U* ptr, Deleter dltr);

    T* get() const noexcept;
    long use_count() const noexcept;

    template<typename U>
    bool owner_before(const SharedPtr<U>& other) const noexcept;

    template<typename U>
    bool owner_before(const WeakPtr<U>& other) const noexcept;
};


#include "../shared_ptr_arr_impl/shared_ptr_arr_constructors.hpp"
#include "../shared_ptr_arr_impl/shared_ptr_arr_operators.hpp"
#include "../shared_ptr_arr_impl/shared_ptr_arr_modifiers.hpp"
#include "../shared_ptr_arr_impl/shared_ptr_arr_observers.hpp"
