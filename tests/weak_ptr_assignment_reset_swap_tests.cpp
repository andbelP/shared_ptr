#include "ptr_test_helpers.hpp"

namespace {

using namespace ptr_tests;

TEST(WeakPtrAssignmentResetSwapTest, AssignWeakSameType) {
    shared_ptr<Tracked> first_owner(new Tracked(1, 32));
    shared_ptr<Tracked> second_owner(new Tracked(101, 33));
    weak_ptr<Tracked> first(first_owner);
    weak_ptr<Tracked> second(second_owner);
    second = first;
    EXPECT_EQ(1, first.use_count());
    EXPECT_EQ(1, second.use_count());
    EXPECT_FALSE(second.expired());
}

TEST(WeakPtrAssignmentResetSwapTest, AssignExpiredWeakMakesTargetExpired) {
    shared_ptr<Tracked> live_owner(new Tracked(1, 40));
    weak_ptr<Tracked> target(live_owner);
    weak_ptr<Tracked> expired;
    {
        shared_ptr<Tracked> temporary(new Tracked(2, 41));
        expired = temporary;
    }
    target = expired;
    EXPECT_TRUE(target.expired());
    EXPECT_EQ(0, target.use_count());
    EXPECT_EQ(1, live_owner.use_count());
}

TEST(WeakPtrAssignmentResetSwapTest, ConvertingAssignWeakDerivedToBase) {
    shared_ptr<Derived> owner(new Derived(1, 34));
    weak_ptr<Derived> derived(owner);
    shared_ptr<Base> empty;
    weak_ptr<Base> base(empty);
    base = derived;
    EXPECT_EQ(1, base.use_count());
    EXPECT_FALSE(base.expired());
}

TEST(WeakPtrAssignmentResetSwapTest, ResetWeakDoesNotDestroyObject) {
    LifetimeLog::Reset();
    shared_ptr<Tracked> owner(new Tracked(1, 35));
    weak_ptr<Tracked> weak(owner);
    weak.reset();
    EXPECT_EQ(1, owner.use_count());
    EXPECT_EQ(0, weak.use_count());
    EXPECT_TRUE(weak.expired());
    EXPECT_EQ(1, LifetimeLog::alive());
    EXPECT_EQ(0, LifetimeLog::destroyed());
}

TEST(WeakPtrAssignmentResetSwapTest, ResetExpiredWeakIsNoOp) {
    weak_ptr<Tracked> weak;
    {
        shared_ptr<Tracked> owner(new Tracked(3, 42));
        weak = owner;
    }
    EXPECT_TRUE(weak.expired());
    weak.reset();
    EXPECT_TRUE(weak.expired());
    EXPECT_EQ(0, weak.use_count());
}

TEST(WeakPtrAssignmentResetSwapTest, SwapTwoLiveWeakPtrs) {
    shared_ptr<Tracked> first_owner(new Tracked(1, 36));
    shared_ptr<Tracked> second_owner(new Tracked(101, 37));
    weak_ptr<Tracked> first(first_owner);
    weak_ptr<Tracked> second(second_owner);
    first.swap(second);
    shared_ptr<Tracked> locked_first = first.lock();
    shared_ptr<Tracked> locked_second = second.lock();
    EXPECT_EQ(second_owner.get(), locked_first.get());
    EXPECT_EQ(first_owner.get(), locked_second.get());
}

TEST(WeakPtrAssignmentResetSwapTest, SwapLiveWithExpiredWeak) {
    shared_ptr<Tracked> owner(new Tracked(4, 43));
    weak_ptr<Tracked> live(owner);
    weak_ptr<Tracked> expired;
    {
        shared_ptr<Tracked> temporary(new Tracked(5, 44));
        expired = temporary;
    }
    live.swap(expired);
    EXPECT_TRUE(live.expired());
    EXPECT_FALSE(expired.expired());
    shared_ptr<Tracked> locked = expired.lock();
    EXPECT_EQ(owner.get(), locked.get());
}

TEST(WeakPtrAssignmentResetSwapTest, AssignmentReturnsSelfReference) {
    shared_ptr<Tracked> first_owner(new Tracked(1, 38));
    shared_ptr<Tracked> second_owner(new Tracked(101, 39));
    weak_ptr<Tracked> first(first_owner);
    weak_ptr<Tracked> second(second_owner);
    weak_ptr<Tracked>* returned = &(second = first);
    EXPECT_EQ(&second, returned);
}

TEST(WeakPtrAssignmentResetSwapTest, SelfAssignmentKeepsWeakTarget) {
    shared_ptr<Tracked> owner(new Tracked(6, 45));
    weak_ptr<Tracked> weak(owner);
    weak = weak;
    EXPECT_FALSE(weak.expired());
    EXPECT_EQ(1, weak.use_count());
    EXPECT_EQ(owner.get(), weak.lock().get());
}

}  // namespace
