#include "ptr_test_helpers.hpp"

namespace {

using namespace ptr_tests;

TEST(SharedPtrObserverTest, DereferenceReadsInt) {
    shared_ptr<int> ptr(new int(10));
    EXPECT_EQ(10, *ptr);
}

TEST(SharedPtrObserverTest, DereferenceWritesInt) {
    shared_ptr<int> ptr(new int(30));
    *ptr = 130;
    EXPECT_EQ(130, *ptr);
}

TEST(SharedPtrObserverTest, GetReturnsStableRawPointer) {
    Tracked* raw = new Tracked(1, 2);
    shared_ptr<Tracked> ptr(raw);
    EXPECT_EQ(raw, ptr.get());
    EXPECT_EQ(raw, ptr.get());
}

TEST(SharedPtrObserverTest, ArrowCallsMemberFunction) {
    shared_ptr<PairPayload> ptr(new PairPayload(1, 11));
    EXPECT_EQ(12, ptr->Sum());
    ptr->SetFirst(6);
    EXPECT_EQ(17, ptr->Sum());
}

TEST(SharedPtrObserverTest, ConstSharedPtrObserversWork) {
    const shared_ptr<Tracked> ptr(new Tracked(20, 30));
    EXPECT_EQ(30, ptr->Get());
    EXPECT_EQ(30, (*ptr).Get());
    EXPECT_NE(nullptr, ptr.get());
    EXPECT_EQ(1, ptr.use_count());
}

}  // namespace
