#include "ptr_test_helpers.hpp"

namespace {

using namespace ptr_tests;

TEST(WeakPtrConstructionTest, DefaultConstructedWeakIsExpired) {
    weak_ptr<Tracked> weak;
    EXPECT_EQ(0, weak.use_count());
    EXPECT_TRUE(weak.expired());
}

TEST(WeakPtrConstructionTest, ConstructFromSharedTracksUseCount) {
    shared_ptr<Tracked> owner(new Tracked(1, 23));
    weak_ptr<Tracked> weak(owner);
    EXPECT_EQ(1, owner.use_count());
    EXPECT_EQ(1, weak.use_count());
    EXPECT_FALSE(weak.expired());
}

TEST(WeakPtrConstructionTest, ConstructFromEmptySharedIsExpired) {
    shared_ptr<Tracked> owner;
    weak_ptr<Tracked> weak(owner);
    EXPECT_EQ(0, weak.use_count());
    EXPECT_TRUE(weak.expired());
}

TEST(WeakPtrConstructionTest, ConvertingConstructFromSharedDerivedToBase) {
    shared_ptr<Derived> owner(new Derived(1, 24));
    weak_ptr<Base> weak(owner);
    EXPECT_EQ(1, weak.use_count());
    EXPECT_FALSE(weak.expired());
}

TEST(WeakPtrConstructionTest, CopyWeakSharesControlBlock) {
    shared_ptr<Tracked> owner(new Tracked(1, 25));
    weak_ptr<Tracked> first(owner);
    weak_ptr<Tracked> second(first);
    EXPECT_EQ(1, first.use_count());
    EXPECT_EQ(1, second.use_count());
    EXPECT_FALSE(second.expired());
}

TEST(WeakPtrConstructionTest, ConvertingCopyWeakDerivedToBase) {
    shared_ptr<Derived> owner(new Derived(1, 26));
    weak_ptr<Derived> derived(owner);
    weak_ptr<Base> base(derived);
    EXPECT_EQ(1, derived.use_count());
    EXPECT_EQ(1, base.use_count());
    EXPECT_FALSE(base.expired());
}

TEST(WeakPtrConstructionTest, CopyExpiredWeakStaysExpired) {
    weak_ptr<Tracked> expired;
    {
        shared_ptr<Tracked> owner(new Tracked(2, 27));
        expired = owner;
    }
    weak_ptr<Tracked> copy(expired);
    EXPECT_TRUE(expired.expired());
    EXPECT_TRUE(copy.expired());
    EXPECT_EQ(0, copy.use_count());
}

TEST(WeakPtrConstructionTest, ConvertingCopyExpiredWeakStaysExpired) {
    weak_ptr<Derived> expired;
    {
        shared_ptr<Derived> owner(new Derived(2, 28));
        expired = owner;
    }
    weak_ptr<Base> copy(expired);
    EXPECT_TRUE(expired.expired());
    EXPECT_TRUE(copy.expired());
    EXPECT_EQ(0, copy.use_count());
}

}  // namespace
