#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#include "control_block.hpp"
#include "default_deleter.hpp"


template<typename T>
class SharedPtr {
    T* data_{};
    ControlBlock* cblock_{};

    template<typename U>
    friend class SharedPtr;

public:

    using element_type = T;
    static_assert(!std::is_array_v<T>); // arrays support will be developed later

    SharedPtr() noexcept;
    SharedPtr(std::nullptr_t) noexcept;
    SharedPtr(const SharedPtr& other);
    SharedPtr(SharedPtr&& other) noexcept;

    template<typename U> requires std::is_convertible_v<U*, T*>
    SharedPtr(const SharedPtr<U>& other);

    template<typename U> requires std::is_convertible_v<U*, T*>
    SharedPtr(SharedPtr<U>&& other) noexcept;

    template<typename Y> requires std::is_convertible_v<Y*, T*>
    explicit SharedPtr(Y* data);

    template<typename Y, typename Deleter> requires std::is_convertible_v<Y*, T*>
    SharedPtr(Y* data, Deleter dltr);

    template<typename Deleter>
    SharedPtr(std::nullptr_t, Deleter dltr);

    template<typename Y>
    SharedPtr(const SharedPtr<Y>& r, element_type* ptr) noexcept;

    ~SharedPtr();

    SharedPtr& operator=(const SharedPtr& other);
    SharedPtr& operator=(SharedPtr&& other) noexcept;

    template<typename U> requires std::is_convertible_v<U*, T*>
    SharedPtr& operator=(const SharedPtr<U>& other);

    template<typename U> requires std::is_convertible_v<U*, T*>
    SharedPtr& operator=(SharedPtr<U>&& other) noexcept;

    T& operator*() const noexcept;
    T* operator->() const noexcept;

    void swap(SharedPtr& other) noexcept;
    void reset() noexcept;

    template<typename U> requires std::is_convertible_v<U*, T*>
    void reset(U* ptr);

    template<typename U, typename Deleter> requires std::is_convertible_v<U*, T*>
    void reset(U* ptr, Deleter dltr);

    T* get() const noexcept;
    std::size_t use_count() const noexcept;
};


#include "shared_ptr_constructors.hpp"
#include "shared_ptr_operators.hpp"
#include "shared_ptr_modifiers.hpp"
#include "shared_ptr_observers.hpp"
