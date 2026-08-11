#pragma once

#include <gtest/gtest.h>

#include "shared_ptr/shared_ptr.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <new>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace shared_ptr_tests {

struct LifetimeStats {
  int constructed = 0;
  int destroyed = 0;
  int copied = 0;
  int moved = 0;
};

struct Tracked {
  explicit Tracked(LifetimeStats* stats = nullptr, int value = 0)
      : stats(stats), value(value) {
    if (stats != nullptr) {
      ++stats->constructed;
    }
  }

  Tracked(const Tracked& other) : stats(other.stats), value(other.value) {
    if (stats != nullptr) {
      ++stats->constructed;
      ++stats->copied;
    }
  }

  Tracked(Tracked&& other) noexcept : stats(other.stats), value(other.value) {
    if (stats != nullptr) {
      ++stats->constructed;
      ++stats->moved;
    }
    other.stats = nullptr;
  }

  virtual ~Tracked() {
    if (stats != nullptr) {
      ++stats->destroyed;
    }
  }

  int Double() const { return value * 2; }

  LifetimeStats* stats;
  int value;
};

struct Base {
  virtual ~Base() = default;
  virtual int Kind() const { return 1; }
  int base_value = 10;
};

struct Derived : Base {
  int Kind() const override { return 2; }
  int derived_value = 20;
};

struct OtherDerived : Base {
  int Kind() const override { return 3; }
};

struct Left {
  virtual ~Left() = default;
  int left = 11;
};

struct Right {
  virtual ~Right() = default;
  int right = 22;
};

struct MultiplyInherited : Left, Right {
  int own = 33;
};

struct Abstract {
  virtual ~Abstract() = default;
  virtual int Value() const = 0;
};

struct Concrete final : Abstract {
  explicit Concrete(int value) : value(value) {}
  int Value() const override { return value; }
  int value;
};

struct CountingDeleter {
  int* calls = nullptr;

  template <class T>
  void operator()(T* pointer) const noexcept {
    if (calls != nullptr) {
      ++*calls;
    }
    delete pointer;
  }

  void operator()(std::nullptr_t) const noexcept {
    if (calls != nullptr) {
      ++*calls;
    }
  }
};

struct ArrayCountingDeleter {
  int* calls = nullptr;

  template <class T>
  void operator()(T* pointer) const noexcept {
    if (calls != nullptr) {
      ++*calls;
    }
    delete[] pointer;
  }
};

struct StatefulDeleter {
  int id = 0;
  int* calls = nullptr;

  template <class T>
  void operator()(T* pointer) const noexcept {
    if (calls != nullptr) {
      ++*calls;
    }
    delete pointer;
  }
};

struct NullAwareDeleter {
  int* calls = nullptr;
  bool* received_null = nullptr;

  template <class T>
  void operator()(T* pointer) const noexcept {
    if (calls != nullptr) {
      ++*calls;
    }
    if (received_null != nullptr) {
      *received_null = pointer == nullptr;
    }
    delete pointer;
  }

  void operator()(std::nullptr_t) const noexcept {
    if (calls != nullptr) {
      ++*calls;
    }
    if (received_null != nullptr) {
      *received_null = true;
    }
  }
};

struct ThrowingConstructor {
  ThrowingConstructor() { throw 42; }
};

struct AllocationStats {
  std::atomic<int> allocations{0};
  std::atomic<int> deallocations{0};
};

template <class T>
struct CountingAllocator {
  using value_type = T;

  CountingAllocator() = default;
  explicit CountingAllocator(AllocationStats* stats) : stats(stats) {}

  template <class U>
  CountingAllocator(const CountingAllocator<U>& other) noexcept : stats(other.stats) {}

  T* allocate(std::size_t count) {
    if (stats != nullptr) {
      stats->allocations.fetch_add(1, std::memory_order_relaxed);
    }
    return std::allocator<T>{}.allocate(count);
  }

  void deallocate(T* pointer, std::size_t count) noexcept {
    if (stats != nullptr) {
      stats->deallocations.fetch_add(1, std::memory_order_relaxed);
    }
    std::allocator<T>{}.deallocate(pointer, count);
  }

  template <class U>
  bool operator==(const CountingAllocator<U>& other) const noexcept {
    return stats == other.stats;
  }

  template <class U>
  bool operator!=(const CountingAllocator<U>& other) const noexcept {
    return !(*this == other);
  }

  AllocationStats* stats = nullptr;
};

template <class T>
struct FailingAllocator {
  using value_type = T;

  FailingAllocator() = default;
  template <class U>
  FailingAllocator(const FailingAllocator<U>&) noexcept {}

  T* allocate(std::size_t) { throw std::bad_alloc(); }
  void deallocate(T*, std::size_t) noexcept {}

  template <class U>
  bool operator==(const FailingAllocator<U>&) const noexcept { return true; }
  template <class U>
  bool operator!=(const FailingAllocator<U>&) const noexcept { return false; }
};

template <class T>
void ExpectEmpty(const SharedPtr<T>& pointer) {
  EXPECT_EQ(pointer.get(), nullptr);
  EXPECT_EQ(pointer.use_count(), 0);
  EXPECT_FALSE(pointer);
}

template <class A, class B>
bool SharesOwnership(const SharedPtr<A>& lhs, const SharedPtr<B>& rhs) {
  return !lhs.owner_before(rhs) && !rhs.owner_before(lhs);
}

template <class A, class B>
bool DifferentOwnership(const SharedPtr<A>& lhs, const SharedPtr<B>& rhs) {
  return lhs.owner_before(rhs) || rhs.owner_before(lhs);
}

}  // namespace shared_ptr_tests
