#include "ptr_test_helpers.hpp"

namespace {

using namespace ptr_tests;

TEST(SharedPtrLifetimeTest, SingleOwnerDeletesOnScopeExit) {
    LifetimeLog::Reset();
    {
        shared_ptr<Tracked> ptr(new Tracked(1, 18));
        EXPECT_EQ(1, LifetimeLog::alive());
    }
    EXPECT_EQ(0, LifetimeLog::alive());
    EXPECT_EQ(1, LifetimeLog::destroyed());
    ASSERT_EQ(1u, LifetimeLog::destroyed_ids().size());
    EXPECT_EQ(1, LifetimeLog::destroyed_ids()[0]);
}

TEST(SharedPtrLifetimeTest, ObjectDiesAfterLastCopy) {
    LifetimeLog::Reset();
    {
        shared_ptr<Tracked> first(new Tracked(1, 19));
        {
            shared_ptr<Tracked> second(first);
            shared_ptr<Tracked> third(second);
            EXPECT_EQ(3, first.use_count());
        }
        EXPECT_EQ(1, LifetimeLog::alive());
        EXPECT_EQ(0, LifetimeLog::destroyed());
    }
    EXPECT_EQ(0, LifetimeLog::alive());
    EXPECT_EQ(1, LifetimeLog::destroyed());
}

TEST(SharedPtrLifetimeTest, PolymorphicDeleteRunsDerivedDestructor) {
    BaseLog::Reset();
    {
        shared_ptr<Base> ptr(new Derived(1, 20));
        EXPECT_EQ(1, BaseLog::base_alive());
        EXPECT_EQ(1, BaseLog::derived_alive());
    }
    EXPECT_EQ(0, BaseLog::base_alive());
    EXPECT_EQ(0, BaseLog::derived_alive());
    EXPECT_EQ(1, BaseLog::base_destroyed());
    EXPECT_EQ(1, BaseLog::derived_destroyed());
}

TEST(SharedPtrLifetimeTest, ReassignmentDoesNotDestroyStillSharedObject) {
    LifetimeLog::Reset();
    shared_ptr<Tracked> first(new Tracked(1, 21));
    shared_ptr<Tracked> first_copy(first);
    shared_ptr<Tracked> replacement(new Tracked(101, 22));
    first = replacement;
    EXPECT_EQ(2, LifetimeLog::alive());
    EXPECT_EQ(0, LifetimeLog::destroyed());
    first_copy = replacement;
    EXPECT_EQ(1, LifetimeLog::alive());
    EXPECT_EQ(1, LifetimeLog::destroyed());
}

}  // namespace
