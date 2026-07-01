#include "ptr_test_helpers.hpp"

namespace {

using namespace ptr_tests;

TEST(SharedPtrResetSwapTest, ResetSingleOwnerDestroysObject) {
    LifetimeLog::Reset();
    shared_ptr<Tracked> ptr(new Tracked(1, 11));
    ptr.reset();
    ExpectEmptyShared(ptr);
    EXPECT_EQ(0, LifetimeLog::alive());
    EXPECT_EQ(1, LifetimeLog::destroyed());
}

TEST(SharedPtrResetSwapTest, ResetEmptyPointerIsNoOp) {
    shared_ptr<Tracked> ptr;
    ptr.reset();
    ExpectEmptyShared(ptr);
}

TEST(SharedPtrResetSwapTest, ResetOneOfTwoOwnersKeepsObjectAlive) {
    LifetimeLog::Reset();
    shared_ptr<Tracked> owner(new Tracked(1, 12));
    shared_ptr<Tracked> copy(owner);
    owner.reset();
    ExpectEmptyShared(owner);
    EXPECT_NE(nullptr, copy.get());
    EXPECT_EQ(1, copy.use_count());
    EXPECT_EQ(1, LifetimeLog::alive());
    EXPECT_EQ(0, LifetimeLog::destroyed());
}

TEST(SharedPtrResetSwapTest, ResetToRawPointerReplacesObject) {
    LifetimeLog::Reset();
    shared_ptr<Tracked> ptr(new Tracked(1, 13));
    Tracked* replacement = new Tracked(101, 14);
    ptr.reset(replacement);
    EXPECT_EQ(replacement, ptr.get());
    EXPECT_EQ(1, ptr.use_count());
    EXPECT_EQ(1, LifetimeLog::alive());
    EXPECT_EQ(1, LifetimeLog::destroyed());
}

TEST(SharedPtrResetSwapTest, ResetToNullRawPointerClearsOwner) {
    LifetimeLog::Reset();
    shared_ptr<Tracked> ptr(new Tracked(1, 15));
    Tracked* null_raw = nullptr;
    ptr.reset(null_raw);
    ExpectEmptyShared(ptr);
    EXPECT_EQ(0, LifetimeLog::alive());
    EXPECT_EQ(1, LifetimeLog::destroyed());
}

TEST(SharedPtrResetSwapTest, ResetBasePointerToDerivedUsesDerivedDeleter) {
    NonVirtualLog::Reset();
    {
        shared_ptr<NonVirtualBase> ptr(new NonVirtualBase(1));
        ptr.reset(new NonVirtualDerived(2, 20));
        EXPECT_EQ(2, ptr->Id());
        EXPECT_EQ(1, NonVirtualLog::base_destroyed());
        EXPECT_EQ(0, NonVirtualLog::derived_destroyed());
    }
    EXPECT_EQ(0, NonVirtualLog::base_alive());
    EXPECT_EQ(0, NonVirtualLog::derived_alive());
    EXPECT_EQ(2, NonVirtualLog::base_destroyed());
    EXPECT_EQ(1, NonVirtualLog::derived_destroyed());
}

TEST(SharedPtrResetSwapTest, SwapOwnerWithEmpty) {
    shared_ptr<Tracked> owner(new Tracked(1, 15));
    shared_ptr<Tracked> empty;
    Tracked* raw = owner.get();
    owner.swap(empty);
    ExpectEmptyShared(owner);
    EXPECT_EQ(raw, empty.get());
    EXPECT_EQ(1, empty.use_count());
}

TEST(SharedPtrResetSwapTest, SwapWithSelfKeepsOwner) {
    shared_ptr<Tracked> owner(new Tracked(1, 16));
    Tracked* raw = owner.get();
    owner.swap(owner);
    EXPECT_EQ(raw, owner.get());
    EXPECT_EQ(1, owner.use_count());
}

TEST(SharedPtrResetSwapTest, SwapTwoOwners) {
    shared_ptr<Tracked> first(new Tracked(1, 16));
    shared_ptr<Tracked> second(new Tracked(101, 17));
    Tracked* first_raw = first.get();
    Tracked* second_raw = second.get();
    first.swap(second);
    EXPECT_EQ(second_raw, first.get());
    EXPECT_EQ(first_raw, second.get());
    EXPECT_EQ(1, first.use_count());
    EXPECT_EQ(1, second.use_count());
}

}  // namespace
