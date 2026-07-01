#include "ptr_test_helpers.hpp"

namespace {

using namespace ptr_tests;

TEST(SharedPtrAssignmentTest, AssignOwnerToEmpty) {
    shared_ptr<Tracked> owner(new Tracked(1, 5));
    shared_ptr<Tracked> target;
    target = owner;
    EXPECT_EQ(owner.get(), target.get());
    EXPECT_EQ(2, owner.use_count());
    EXPECT_EQ(2, target.use_count());
}

TEST(SharedPtrAssignmentTest, AssignEmptyToOwnerReleasesObject) {
    LifetimeLog::Reset();
    shared_ptr<Tracked> target(new Tracked(1, 6));
    shared_ptr<Tracked> empty;
    target = empty;
    ExpectEmptyShared(target);
    EXPECT_EQ(0, LifetimeLog::alive());
    EXPECT_EQ(1, LifetimeLog::destroyed());
}

TEST(SharedPtrAssignmentTest, SelfAssignmentKeepsObject) {
    LifetimeLog::Reset();
    shared_ptr<Tracked> ptr(new Tracked(1, 7));
    Tracked* raw = ptr.get();
    ptr = ptr;
    EXPECT_EQ(raw, ptr.get());
    EXPECT_EQ(1, ptr.use_count());
    EXPECT_EQ(1, LifetimeLog::alive());
    EXPECT_EQ(0, LifetimeLog::destroyed());
}

TEST(SharedPtrAssignmentTest, AssignReplacementDestroysOldObject) {
    LifetimeLog::Reset();
    shared_ptr<Tracked> target(new Tracked(1, 8));
    shared_ptr<Tracked> replacement(new Tracked(101, 9));
    Tracked* replacement_raw = replacement.get();
    target = replacement;
    EXPECT_EQ(replacement_raw, target.get());
    EXPECT_EQ(2, replacement.use_count());
    EXPECT_EQ(1, LifetimeLog::destroyed());
}

TEST(SharedPtrAssignmentTest, ConvertingAssignmentDerivedToBase) {
    shared_ptr<Derived> derived(new Derived(1, 10));
    shared_ptr<Base> base;
    base = derived;
    EXPECT_EQ(derived.get(), base.get());
    EXPECT_EQ(2, derived.use_count());
    EXPECT_EQ(2, base.use_count());
    EXPECT_EQ(2, base->Kind());
}

TEST(SharedPtrAssignmentTest, ConvertingAssignmentKeepsOriginalDerivedDeleter) {
    NonVirtualLog::Reset();
    shared_ptr<NonVirtualBase> base;
    {
        shared_ptr<NonVirtualDerived> derived(new NonVirtualDerived(2, 20));
        base = derived;
        EXPECT_EQ(derived.get(), base.get());
        EXPECT_EQ(2, derived.use_count());
        EXPECT_EQ(2, base.use_count());
    }
    EXPECT_EQ(1, NonVirtualLog::base_alive());
    EXPECT_EQ(1, NonVirtualLog::derived_alive());
    base.reset();
    EXPECT_EQ(0, NonVirtualLog::base_alive());
    EXPECT_EQ(0, NonVirtualLog::derived_alive());
    EXPECT_EQ(1, NonVirtualLog::base_destroyed());
    EXPECT_EQ(1, NonVirtualLog::derived_destroyed());
}

TEST(SharedPtrAssignmentTest, AssignEmptyToOneOfSeveralOwnersKeepsObjectAlive) {
    LifetimeLog::Reset();
    shared_ptr<Tracked> owner(new Tracked(3, 30));
    shared_ptr<Tracked> copy(owner);
    shared_ptr<Tracked> empty;
    owner = empty;
    ExpectEmptyShared(owner);
    EXPECT_EQ(1, copy.use_count());
    EXPECT_EQ(1, LifetimeLog::alive());
    EXPECT_EQ(0, LifetimeLog::destroyed());
}

}  // namespace
