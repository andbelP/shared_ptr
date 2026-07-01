#pragma once

#include "shared_ptr.hpp"
#include "weak_ptr.hpp"

#include <gtest/gtest.h>

#include <vector>

namespace ptr_tests {

struct LifetimeLog {
    static int& alive() {
        static int value = 0;
        return value;
    }

    static int& created() {
        static int value = 0;
        return value;
    }

    static int& destroyed() {
        static int value = 0;
        return value;
    }

    static std::vector<int>& destroyed_ids() {
        static std::vector<int> value;
        return value;
    }

    static void Reset() {
        alive() = 0;
        created() = 0;
        destroyed() = 0;
        destroyed_ids().clear();
    }
};

struct Tracked {
    int id;
    int value;

    explicit Tracked(int input_id = 0, int input_value = 0)
        : id(input_id), value(input_value) {
        ++LifetimeLog::alive();
        ++LifetimeLog::created();
    }

    ~Tracked() {
        --LifetimeLog::alive();
        ++LifetimeLog::destroyed();
        LifetimeLog::destroyed_ids().push_back(id);
    }

    int Get() const noexcept { return value; }
    void Set(int input_value) noexcept { value = input_value; }
    int Sum(int add) const noexcept { return value + add; }
    Tracked* Self() noexcept { return this; }
    const Tracked* Self() const noexcept { return this; }
};

struct PairPayload {
    int first;
    int second;

    PairPayload(int input_first = 0, int input_second = 0)
        : first(input_first), second(input_second) {}

    int Sum() const noexcept { return first + second; }
    void SetFirst(int value) noexcept { first = value; }
    void SetSecond(int value) noexcept { second = value; }
};

struct BaseLog {
    static int& base_alive() {
        static int value = 0;
        return value;
    }

    static int& base_destroyed() {
        static int value = 0;
        return value;
    }

    static int& derived_alive() {
        static int value = 0;
        return value;
    }

    static int& derived_destroyed() {
        static int value = 0;
        return value;
    }

    static void Reset() {
        base_alive() = 0;
        base_destroyed() = 0;
        derived_alive() = 0;
        derived_destroyed() = 0;
    }
};

struct Base {
    int id;

    explicit Base(int input_id = 0) : id(input_id) { ++BaseLog::base_alive(); }
    virtual ~Base() {
        --BaseLog::base_alive();
        ++BaseLog::base_destroyed();
    }

    virtual int Kind() const noexcept { return 1; }
    int Id() const noexcept { return id; }
};

struct Derived : Base {
    int extra;

    explicit Derived(int input_id = 0, int input_extra = 0)
        : Base(input_id), extra(input_extra) {
        ++BaseLog::derived_alive();
    }

    ~Derived() override {
        --BaseLog::derived_alive();
        ++BaseLog::derived_destroyed();
    }

    int Kind() const noexcept override { return 2; }
    int Extra() const noexcept { return extra; }
};

struct NonVirtualLog {
    static int& base_alive() {
        static int value = 0;
        return value;
    }

    static int& base_destroyed() {
        static int value = 0;
        return value;
    }

    static int& derived_alive() {
        static int value = 0;
        return value;
    }

    static int& derived_destroyed() {
        static int value = 0;
        return value;
    }

    static void Reset() {
        base_alive() = 0;
        base_destroyed() = 0;
        derived_alive() = 0;
        derived_destroyed() = 0;
    }
};

struct NonVirtualBase {
    int id;

    explicit NonVirtualBase(int input_id = 0) : id(input_id) {
        ++NonVirtualLog::base_alive();
    }

    ~NonVirtualBase() {
        --NonVirtualLog::base_alive();
        ++NonVirtualLog::base_destroyed();
    }

    int Id() const noexcept { return id; }
};

struct NonVirtualDerived : NonVirtualBase {
    int extra;

    explicit NonVirtualDerived(int input_id = 0, int input_extra = 0)
        : NonVirtualBase(input_id), extra(input_extra) {
        ++NonVirtualLog::derived_alive();
    }

    ~NonVirtualDerived() {
        --NonVirtualLog::derived_alive();
        ++NonVirtualLog::derived_destroyed();
    }

    int Extra() const noexcept { return extra; }
};

struct NoDefault {
    int value;

    explicit NoDefault(int input_value) : value(input_value) {}
    int Get() const noexcept { return value; }
};

template <typename T>
void ExpectEmptyShared(const shared_ptr<T>& ptr) {
    EXPECT_EQ(nullptr, ptr.get());
    EXPECT_EQ(0, ptr.use_count());
}

template <typename T>
void ExpectOwner(const shared_ptr<T>& ptr, T* raw, long count) {
    EXPECT_EQ(raw, ptr.get());
    EXPECT_EQ(count, ptr.use_count());
}

}  // namespace ptr_tests
