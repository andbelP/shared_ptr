#include "ptr_test_helpers.hpp"

namespace {

using namespace ptr_tests;

TEST(SharedPtrCopyTest, CopyConstructionSharesOwnership) {
    shared_ptr<Tracked> first(new Tracked(1, 2));
    shared_ptr<Tracked> second(first);
    EXPECT_EQ(first.get(), second.get());
    EXPECT_EQ(2, first.use_count());
    EXPECT_EQ(2, second.use_count());
}

TEST(SharedPtrCopyTest, CopyFromEmptyStaysEmpty) {
    shared_ptr<Tracked> empty;
    shared_ptr<Tracked> copy(empty);
    ExpectEmptyShared(empty);
    ExpectEmptyShared(copy);
}

TEST(SharedPtrCopyTest, NestedCopyDropsCountAfterScope) {
    shared_ptr<Tracked> owner(new Tracked(1, 3));
    {
        shared_ptr<Tracked> copy(owner);
        EXPECT_EQ(2, owner.use_count());
    }
    EXPECT_EQ(1, owner.use_count());
    EXPECT_NE(nullptr, owner.get());
}

TEST(SharedPtrCopyTest, MultipleCopiesReportSameCount) {
    shared_ptr<Tracked> a(new Tracked(1, 4));
    shared_ptr<Tracked> b(a);
    shared_ptr<Tracked> c(b);
    shared_ptr<Tracked> d(a);
    EXPECT_EQ(4, a.use_count());
    EXPECT_EQ(4, b.use_count());
    EXPECT_EQ(4, c.use_count());
    EXPECT_EQ(4, d.use_count());
    EXPECT_EQ(a.get(), d.get());
}

TEST(SharedPtrCopyTest, ConvertingCopyDerivedToBase) {
    BaseLog::Reset();
    shared_ptr<Derived> derived(new Derived(1, 20));
    shared_ptr<Base> base(derived);
    EXPECT_EQ(derived.get(), base.get());
    EXPECT_EQ(2, derived.use_count());
    EXPECT_EQ(2, base.use_count());
    EXPECT_EQ(2, base->Kind());
}

TEST(SharedPtrCopyTest, ConvertingCopyKeepsOriginalDerivedDeleter) {
    NonVirtualLog::Reset();
    {
        shared_ptr<NonVirtualDerived> derived(new NonVirtualDerived(2, 22));
        {
            shared_ptr<NonVirtualBase> base(derived);
            EXPECT_EQ(derived.get(), base.get());
            EXPECT_EQ(2, derived.use_count());
            EXPECT_EQ(2, base.use_count());
            EXPECT_EQ(1, NonVirtualLog::derived_alive());
        }
        EXPECT_EQ(1, NonVirtualLog::derived_alive());
        EXPECT_EQ(0, NonVirtualLog::derived_destroyed());
    }
    EXPECT_EQ(0, NonVirtualLog::base_alive());
    EXPECT_EQ(0, NonVirtualLog::derived_alive());
    EXPECT_EQ(1, NonVirtualLog::base_destroyed());
    EXPECT_EQ(1, NonVirtualLog::derived_destroyed());
}

}  // namespace
