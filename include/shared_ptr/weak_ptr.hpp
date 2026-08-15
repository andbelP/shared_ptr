#pragma once

#include "../control_block/control_block.hpp"
#include "shared_ptr.hpp"


template<typename T>
class WeakPtr{
    ControlBlock* cblock_{};

    template<typename U>
    friend class WeakPtr;
    
public:

    constexpr WeakPtr() noexcept = default;

    template<typename U> requires std::is_convertible_v<U*, T*>
    WeakPtr(const SharedPtr<U>& ptr) noexcept;

    WeakPtr(const WeakPtr& other) noexcept;

    template<typename U> requires std::is_convertible_v<U*, T*>
    WeakPtr(const WeakPtr<U>& other) noexcept;

    WeakPtr(WeakPtr&& other) noexcept;

    template<typename U> requires std::is_convertible_v<U*, T*>
    WeakPtr(WeakPtr<U>&& other) noexcept;

    ~WeakPtr();

    void swap(WeakPtr& other) noexcept;

    WeakPtr& operator=(const WeakPtr& other) noexcept;

    template<typename U> requires std::is_convertible_v<U*, T*>
    WeakPtr& operator=(const WeakPtr<U>& other) noexcept;

    template<typename U> requires std::is_convertible_v<U*, T*>
    WeakPtr& operator=(const SharedPtr<U>& other) noexcept;

    template<typename U> requires std::is_convertible_v<U*, T*>
    WeakPtr& operator=(WeakPtr<U>&& other) noexcept;
    
    WeakPtr& operator=(WeakPtr&& other) noexcept;

};


#include "weak_ptr_impl/weak_ptr_constructors.hpp"
#include "weak_ptr_impl/weak_ptr_modifiers.hpp"
#include "weak_ptr_impl/weak_ptr_operators.hpp"