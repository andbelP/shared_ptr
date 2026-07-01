#include "ptr_test_helpers.hpp"

namespace {

using namespace ptr_tests;

TEST(SharedWeakInteractionTest, SharedResetExpiresWeak) {
    shared_ptr<Tracked> owner(new Tracked(1, 40));
    weak_ptr<Tracked> weak(owner);
    owner.reset();
    EXPECT_EQ(0, weak.use_count());
    EXPECT_TRUE(weak.expired());
}

TEST(SharedWeakInteractionTest, SharedConstructedFromWeakKeepsObjectAlive) {
    LifetimeLog::Reset();
    shared_ptr<Tracked> owner(new Tracked(1, 44));
    weak_ptr<Tracked> weak(owner);
    {
        shared_ptr<Tracked> from_weak(weak);
        owner.reset();
        EXPECT_EQ(1, from_weak.use_count());
        EXPECT_FALSE(weak.expired());
        EXPECT_EQ(1, LifetimeLog::alive());
    }
    EXPECT_TRUE(weak.expired());
    EXPECT_EQ(0, LifetimeLog::alive());
    EXPECT_EQ(1, LifetimeLog::destroyed());
}

TEST(SharedWeakInteractionTest, LockedSharedKeepsObjectAliveAfterOriginalReset) {
    LifetimeLog::Reset();
    shared_ptr<Tracked> owner(new Tracked(1, 41));
    weak_ptr<Tracked> weak(owner);
    shared_ptr<Tracked> locked = weak.lock();
    owner.reset();
    EXPECT_EQ(1, locked.use_count());
    EXPECT_FALSE(weak.expired());
    EXPECT_EQ(1, LifetimeLog::alive());
}

TEST(SharedWeakInteractionTest, DestroyingWeakBeforeSharedDoesNotAffectOwner) {
    LifetimeLog::Reset();
    shared_ptr<Tracked> owner(new Tracked(1, 46));
    {
        weak_ptr<Tracked> weak(owner);
        EXPECT_EQ(1, owner.use_count());
        EXPECT_EQ(1, weak.use_count());
    }
    EXPECT_EQ(1, owner.use_count());
    EXPECT_EQ(1, LifetimeLog::alive());
    EXPECT_EQ(0, LifetimeLog::destroyed());
}

TEST(SharedWeakInteractionTest, MultipleWeakPtrsDoNotIncreaseSharedUseCount) {
    shared_ptr<Tracked> owner(new Tracked(1, 42));
    weak_ptr<Tracked> weak_a(owner);
    weak_ptr<Tracked> weak_b(owner);
    weak_ptr<Tracked> weak_c(owner);
    EXPECT_EQ(1, owner.use_count());
    EXPECT_EQ(1, weak_a.use_count());
    EXPECT_EQ(1, weak_b.use_count());
    EXPECT_EQ(1, weak_c.use_count());
}

TEST(SharedWeakInteractionTest, WeakOutlivesSharedAndCanBeDestroyedExpired) {
    LifetimeLog::Reset();
    weak_ptr<Tracked> weak;
    {
        shared_ptr<Tracked> owner(new Tracked(1, 47));
        weak = owner;
        EXPECT_FALSE(weak.expired());
    }
    EXPECT_TRUE(weak.expired());
    EXPECT_EQ(0, weak.use_count());
    EXPECT_EQ(0, LifetimeLog::alive());
    EXPECT_EQ(1, LifetimeLog::destroyed());
    weak.reset();
    EXPECT_TRUE(weak.expired());
}

TEST(SharedWeakInteractionTest, WeakUseCountFollowsSharedAssignment) {
    shared_ptr<Tracked> owner(new Tracked(1, 43));
    weak_ptr<Tracked> weak(owner);
    shared_ptr<Tracked> copy;
    copy = owner;
    EXPECT_EQ(2, weak.use_count());
    copy.reset();
    EXPECT_EQ(1, weak.use_count());
}

TEST(SharedWeakInteractionTest, BadWeakPtrWhatReturnsMessage) {
    bad_weak_ptr error("expired weak pointer 0");
    EXPECT_STREQ("expired weak pointer 0", error.what());
}

}  // namespace
