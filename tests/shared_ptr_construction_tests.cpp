#include "ptr_test_helpers.hpp"

namespace {

using namespace ptr_tests;

TEST(SharedPtrConstructionTest, DefaultConstructedIntIsEmpty) {
    shared_ptr<int> ptr;
    ExpectEmptyShared(ptr);
}

TEST(SharedPtrConstructionTest, DefaultConstructedTrackedIsEmpty) {
    shared_ptr<Tracked> ptr;
    ExpectEmptyShared(ptr);
    EXPECT_EQ(0, LifetimeLog::alive());
}

TEST(SharedPtrConstructionTest, NullptrConstructedIsEmpty) {
    shared_ptr<Tracked> ptr(nullptr);
    ExpectEmptyShared(ptr);
    EXPECT_EQ(0, LifetimeLog::alive());
}

TEST(SharedPtrConstructionTest, NullRawPointerIsEmpty) {
    Tracked* raw = nullptr;
    shared_ptr<Tracked> ptr(raw);
    ExpectEmptyShared(ptr);
    EXPECT_EQ(0, LifetimeLog::alive());
}

TEST(SharedPtrConstructionTest, RawIntPointerOwnsValue) {
    int* raw = new int(100);
    shared_ptr<int> ptr(raw);
    ExpectOwner(ptr, raw, 1);
    EXPECT_EQ(100, *ptr);
}

TEST(SharedPtrConstructionTest, RawTrackedPointerOwnsObject) {
    LifetimeLog::Reset();
    Tracked* raw = new Tracked(1, 200);
    shared_ptr<Tracked> ptr(raw);
    ExpectOwner(ptr, raw, 1);
    EXPECT_EQ(1, ptr.get()->id);
    EXPECT_EQ(200, (*ptr).value);
}

TEST(SharedPtrConstructionTest, DirectBaseConstructionFromDerivedUsesDerivedDeleter) {
    NonVirtualLog::Reset();
    {
        shared_ptr<NonVirtualBase> ptr(new NonVirtualDerived(7, 70));
        EXPECT_EQ(7, ptr->Id());
        EXPECT_EQ(1, NonVirtualLog::base_alive());
        EXPECT_EQ(1, NonVirtualLog::derived_alive());
    }
    EXPECT_EQ(0, NonVirtualLog::base_alive());
    EXPECT_EQ(0, NonVirtualLog::derived_alive());
    EXPECT_EQ(1, NonVirtualLog::base_destroyed());
    EXPECT_EQ(1, NonVirtualLog::derived_destroyed());
}

TEST(SharedPtrConstructionTest, ConstructFromLiveWeakSharesOwnership) {
    shared_ptr<Tracked> owner(new Tracked(3, 300));
    weak_ptr<Tracked> weak(owner);
    shared_ptr<Tracked> from_weak(weak);
    EXPECT_EQ(owner.get(), from_weak.get());
    EXPECT_EQ(2, owner.use_count());
    EXPECT_EQ(2, from_weak.use_count());
    EXPECT_FALSE(weak.expired());
}

TEST(SharedPtrConstructionTest, ConstructFromExpiredWeakThrows) {
    weak_ptr<Tracked> weak;
    {
        shared_ptr<Tracked> owner(new Tracked(4, 400));
        weak = owner;
    }
    EXPECT_TRUE(weak.expired());
    EXPECT_THROW(shared_ptr<Tracked> from_weak(weak), bad_weak_ptr);
}

TEST(SharedPtrConstructionTest, ConvertingConstructFromWeakDerivedToBase) {
    shared_ptr<Derived> owner(new Derived(5, 500));
    weak_ptr<Derived> weak(owner);
    shared_ptr<Base> base(weak);
    EXPECT_EQ(owner.get(), base.get());
    EXPECT_EQ(2, owner.use_count());
    EXPECT_EQ(2, base.use_count());
    EXPECT_EQ(2, base->Kind());
}

}  // namespace
