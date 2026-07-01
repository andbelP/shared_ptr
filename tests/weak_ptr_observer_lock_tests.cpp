#include "ptr_test_helpers.hpp"

#include <type_traits>
#include <utility>

namespace {

using namespace ptr_tests;

template <typename T>
class HasConstWeakLock {
    template <typename U>
    static auto Test(int)
        -> decltype(std::declval<const weak_ptr<U>&>().lock(), std::true_type());

    template <typename>
    static std::false_type Test(...);

   public:
    static bool Value() { return decltype(Test<T>(0))::value; }
};

TEST(WeakPtrObserverLockTest, ExpiredFalseWhileOwnerAlive) {
    shared_ptr<Tracked> owner(new Tracked(1, 27));
    weak_ptr<Tracked> weak(owner);
    EXPECT_FALSE(weak.expired());
    EXPECT_EQ(1, weak.use_count());
}

TEST(WeakPtrObserverLockTest, ExpiredTrueAfterTemporaryOwnerDies) {
    weak_ptr<Tracked> weak(shared_ptr<Tracked>(new Tracked(1, 28)));
    EXPECT_EQ(0, weak.use_count());
    EXPECT_TRUE(weak.expired());
}

TEST(WeakPtrObserverLockTest, LockLiveWeakReturnsSharedOwner) {
    shared_ptr<Tracked> owner(new Tracked(1, 29));
    weak_ptr<Tracked> weak(owner);
    shared_ptr<Tracked> locked = weak.lock();
    EXPECT_EQ(owner.get(), locked.get());
    EXPECT_EQ(2, owner.use_count());
    EXPECT_EQ(2, locked.use_count());
}

TEST(WeakPtrObserverLockTest, LockTwiceCreatesSeparateSharedOwners) {
    shared_ptr<Tracked> owner(new Tracked(2, 29));
    weak_ptr<Tracked> weak(owner);
    shared_ptr<Tracked> first = weak.lock();
    EXPECT_EQ(2, owner.use_count());
    shared_ptr<Tracked> second = weak.lock();
    EXPECT_EQ(owner.get(), first.get());
    EXPECT_EQ(owner.get(), second.get());
    EXPECT_EQ(3, owner.use_count());
    first.reset();
    EXPECT_EQ(2, owner.use_count());
    second.reset();
    EXPECT_EQ(1, owner.use_count());
}

TEST(WeakPtrObserverLockTest, LockExpiredWeakReturnsEmpty) {
    weak_ptr<Tracked> weak(shared_ptr<Tracked>(new Tracked(1, 30)));
    shared_ptr<Tracked> locked = weak.lock();
    ExpectEmptyShared(locked);
    EXPECT_TRUE(weak.expired());
}

TEST(WeakPtrObserverLockTest, LockAfterOwnerResetReturnsEmpty) {
    shared_ptr<Tracked> owner(new Tracked(2, 30));
    weak_ptr<Tracked> weak(owner);
    owner.reset();
    shared_ptr<Tracked> locked = weak.lock();
    ExpectEmptyShared(locked);
    EXPECT_TRUE(weak.expired());
    EXPECT_EQ(0, weak.use_count());
}

TEST(WeakPtrObserverLockTest, UseCountTracksSharedCopies) {
    shared_ptr<Tracked> owner(new Tracked(1, 31));
    weak_ptr<Tracked> weak(owner);
    {
        shared_ptr<Tracked> copy(owner);
        EXPECT_EQ(2, weak.use_count());
    }
    EXPECT_EQ(1, weak.use_count());
}

TEST(WeakPtrObserverLockTest, ConstWeakPtrSupportsLock) {
    EXPECT_TRUE(HasConstWeakLock<Tracked>::Value());
}

}  // namespace
