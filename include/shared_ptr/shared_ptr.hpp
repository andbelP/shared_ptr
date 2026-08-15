#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#include "../control_block/control_block.hpp"


template<typename T>
class SharedPtr {
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

    template<typename Y, typename Deleter> requires std::is_convertible_v<Y*, T*>
    SharedPtr(std::unique_ptr<Y, Deleter>&& ptr);

    ~SharedPtr();

    SharedPtr& operator=(const SharedPtr& other);
    SharedPtr& operator=(SharedPtr&& other) noexcept;

    template<typename U> requires std::is_convertible_v<U*, T*>
    SharedPtr& operator=(const SharedPtr<U>& other);

    template<typename U> requires std::is_convertible_v<U*, T*>
    SharedPtr& operator=(SharedPtr<U>&& other) noexcept;

    T& operator*() const noexcept;
    T* operator->() const noexcept;

    explicit operator bool() const noexcept;

    void swap(SharedPtr& other) noexcept;
    void reset() noexcept;

    template<typename U> requires std::is_convertible_v<U*, T*>
    void reset(U* ptr);

    template<typename U, typename Deleter> requires std::is_convertible_v<U*, T*>
    void reset(U* ptr, Deleter dltr);

    T* get() const noexcept;
    long use_count() const noexcept;

    template<typename U>
    bool owner_before(const SharedPtr<U>& other) const noexcept;
    // TODO:: add owner_before for WeakPtr
};


#include "shared_ptr_impl/shared_ptr_constructors.hpp"
#include "shared_ptr_impl/shared_ptr_operators.hpp"
#include "shared_ptr_impl/shared_ptr_modifiers.hpp"
#include "shared_ptr_impl/shared_ptr_observers.hpp"
