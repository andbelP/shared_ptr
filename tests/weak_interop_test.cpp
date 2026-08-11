#include "test_support.hpp"

#include "shared_ptr/weak_ptr.hpp"

namespace shared_ptr_tests {

TEST(SharedPtrWeakType, MatchesWeakPtrOfElementType) {
  static_assert(std::is_same_v<typename SharedPtr<int>::weak_type, WeakPtr<int>>);
  static_assert(std::is_same_v<typename SharedPtr<const Base>::weak_type,
                               WeakPtr<const Base>>);
}

TEST(SharedPtrWeakConstructor, LocksLiveOwner) {
  SharedPtr<int> owner(new int(42));
  WeakPtr<int> weak(owner);
  SharedPtr<int> locked(weak);
  EXPECT_EQ(locked.get(), owner.get());
  EXPECT_EQ(*locked, 42);
}

TEST(SharedPtrWeakConstructor, IncrementsStrongCount) {
  SharedPtr<int> owner(new int(42));
  WeakPtr<int> weak(owner);
  SharedPtr<int> locked(weak);
  EXPECT_EQ(owner.use_count(), 2);
  EXPECT_EQ(locked.use_count(), 2);
}

TEST(SharedPtrWeakConstructor, ThrowsForExpiredWeakPointer) {
  WeakPtr<int> weak;
  {
    SharedPtr<int> owner(new int(42));
    weak = owner;
  }
  EXPECT_THROW((SharedPtr<int>(weak)), std::bad_weak_ptr);
}

TEST(SharedPtrWeakConstructor, ThrowsForDefaultWeakPointer) {
  WeakPtr<int> weak;
  EXPECT_THROW((SharedPtr<int>(weak)), std::bad_weak_ptr);
}

TEST(SharedPtrWeakConstructor, ConvertsDerivedWeakToBaseShared) {
  SharedPtr<Derived> owner(new Derived);
  WeakPtr<Derived> weak(owner);
  SharedPtr<Base> locked(weak);
  EXPECT_EQ(locked->Kind(), 2);
  EXPECT_EQ(owner.use_count(), 2);
}

TEST(SharedPtrWeakConstructor, AddsConstQualification) {
  SharedPtr<int> owner(new int(8));
  WeakPtr<int> weak(owner);
  SharedPtr<const int> locked(weak);
  EXPECT_EQ(*locked, 8);
}

TEST(SharedPtrWeakConstructor, PreservesCustomDeleter) {
  int calls = 0;
  WeakPtr<int> weak;
  {
    SharedPtr<int> owner(new int, CountingDeleter{&calls});
    weak = owner;
    SharedPtr<int> locked(weak);
    owner.reset();
    EXPECT_EQ(calls, 0);
  }
  EXPECT_EQ(calls, 1);
}

TEST(SharedPtrWeakConstructor, PreservesOwnerIdentity) {
  SharedPtr<int> owner(new int);
  WeakPtr<int> weak(owner);
  SharedPtr<int> locked(weak);
  EXPECT_TRUE(SharesOwnership(owner, locked));
}

TEST(SharedPtrWeakLock, ReturnsSharedOwnerWhileAlive) {
  SharedPtr<int> owner(new int(16));
  WeakPtr<int> weak(owner);
  SharedPtr<int> locked = weak.lock();
  EXPECT_EQ(*locked, 16);
  EXPECT_EQ(owner.use_count(), 2);
}

TEST(SharedPtrWeakLock, ReturnsEmptyAfterExpiration) {
  WeakPtr<int> weak;
  {
    SharedPtr<int> owner(new int(16));
    weak = owner;
  }
  SharedPtr<int> locked = weak.lock();
  ExpectEmpty(locked);
}

TEST(SharedPtrWeakLifetime, WeakOwnerDoesNotKeepObjectAlive) {
  LifetimeStats stats;
  WeakPtr<Tracked> weak;
  {
    SharedPtr<Tracked> owner(new Tracked(&stats));
    weak = owner;
  }
  EXPECT_EQ(stats.destroyed, 1);
  EXPECT_TRUE(weak.expired());
}

TEST(SharedPtrWeakLifetime, SharedFromWeakKeepsObjectAlive) {
  LifetimeStats stats;
  WeakPtr<Tracked> weak;
  SharedPtr<Tracked> locked;
  {
    SharedPtr<Tracked> owner(new Tracked(&stats));
    weak = owner;
    locked = weak.lock();
  }
  EXPECT_EQ(stats.destroyed, 0);
  locked.reset();
  EXPECT_EQ(stats.destroyed, 1);
}

TEST(SharedPtrWeakUseCount, TracksStrongOwners) {
  SharedPtr<int> first(new int);
  WeakPtr<int> weak(first);
  EXPECT_EQ(weak.use_count(), 1);
  SharedPtr<int> second(first);
  EXPECT_EQ(weak.use_count(), 2);
  second.reset();
  EXPECT_EQ(weak.use_count(), 1);
}

TEST(SharedPtrWeakUseCount, BecomesZeroAfterLastStrongOwner) {
  WeakPtr<int> weak;
  {
    SharedPtr<int> owner(new int);
    weak = owner;
  }
  EXPECT_EQ(weak.use_count(), 0);
}

TEST(SharedPtrWeakInterop, ResetLockedOwnerUpdatesWeakCount) {
  SharedPtr<int> owner(new int);
  WeakPtr<int> weak(owner);
  SharedPtr<int> locked = weak.lock();
  owner.reset();
  EXPECT_FALSE(weak.expired());
  EXPECT_EQ(weak.use_count(), 1);
  locked.reset();
  EXPECT_TRUE(weak.expired());
}

TEST(SharedPtrWeakInterop, AliasingOwnerCanProduceEquivalentWeakOwner) {
  struct Pair { int a; int b; };
  SharedPtr<Pair> owner(new Pair{1, 2});
  SharedPtr<int> alias(owner, &owner->b);
  WeakPtr<int> weak(alias);
  auto locked = weak.lock();
  EXPECT_EQ(locked.get(), &owner->b);
  EXPECT_TRUE(SharesOwnership(owner, locked));
}

TEST(SharedPtrWeakInterop, WeakOwnerBeforeAgreesWithSharedOwnerBefore) {
  SharedPtr<int> first(new int);
  SharedPtr<int> second(new int);
  WeakPtr<int> weak_first(first);
  EXPECT_EQ(weak_first.owner_before(second), first.owner_before(second));
  EXPECT_EQ(second.owner_before(weak_first), second.owner_before(first));
}

TEST(SharedPtrWeakInterop, ConvertingWeakPreservesPolymorphism) {
  SharedPtr<Derived> derived(new Derived);
  WeakPtr<Base> weak(derived);
  auto base = weak.lock();
  ASSERT_TRUE(base);
  EXPECT_EQ(base->Kind(), 2);
}

TEST(SharedPtrWeakInterop, EmptyWeakOwnerBeforeMatchesEmptyShared) {
  WeakPtr<int> weak;
  SharedPtr<int> shared;
  EXPECT_FALSE(weak.owner_before(shared));
  EXPECT_FALSE(shared.owner_before(weak));
}

TEST(SharedPtrWeakInterop, WeakResetDoesNotAffectStrongOwner) {
  SharedPtr<int> owner(new int(5));
  WeakPtr<int> weak(owner);
  weak.reset();
  EXPECT_EQ(owner.use_count(), 1);
  EXPECT_EQ(*owner, 5);
}

}  // namespace shared_ptr_tests
