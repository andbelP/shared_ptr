#include "test_support.hpp"

namespace shared_ptr_tests {

TEST(SharedPtrReset, EmptyPointerRemainsEmpty) {
  SharedPtr<int> pointer;
  pointer.reset();
  ExpectEmpty(pointer);
}

TEST(SharedPtrReset, ReleasesSoleOwnedObject) {
  LifetimeStats stats;
  SharedPtr<Tracked> pointer(new Tracked(&stats));
  pointer.reset();
  EXPECT_EQ(stats.destroyed, 1);
  ExpectEmpty(pointer);
}

TEST(SharedPtrReset, ReleasesOneSharedOwner) {
  SharedPtr<int> first(new int(3));
  SharedPtr<int> second(first);
  first.reset();
  ExpectEmpty(first);
  EXPECT_EQ(second.use_count(), 1);
  EXPECT_EQ(*second, 3);
}

TEST(SharedPtrReset, WithoutArgumentIsNoexcept) {
  static_assert(noexcept(std::declval<SharedPtr<int>&>().reset()));
}

TEST(SharedPtrResetRaw, ReplacesManagedObject) {
  SharedPtr<int> pointer(new int(1));
  int* replacement = new int(2);
  pointer.reset(replacement);
  EXPECT_EQ(pointer.get(), replacement);
  EXPECT_EQ(*pointer, 2);
}

TEST(SharedPtrResetRaw, NewObjectHasOneOwner) {
  SharedPtr<int> pointer(new int(1));
  pointer.reset(new int(2));
  EXPECT_EQ(pointer.use_count(), 1);
}

TEST(SharedPtrResetRaw, ReleasesPreviousObject) {
  LifetimeStats first_stats;
  LifetimeStats second_stats;
  SharedPtr<Tracked> pointer(new Tracked(&first_stats));
  pointer.reset(new Tracked(&second_stats));
  EXPECT_EQ(first_stats.destroyed, 1);
  EXPECT_EQ(second_stats.destroyed, 0);
}

TEST(SharedPtrResetRaw, DoesNotAffectOtherOwners) {
  SharedPtr<int> first(new int(1));
  SharedPtr<int> second(first);
  first.reset(new int(2));
  EXPECT_EQ(*first, 2);
  EXPECT_EQ(*second, 1);
  EXPECT_EQ(first.use_count(), 1);
  EXPECT_EQ(second.use_count(), 1);
}

TEST(SharedPtrResetRaw, AcceptsDerivedForBase) {
  SharedPtr<Base> pointer;
  pointer.reset(new Derived);
  EXPECT_EQ(pointer->Kind(), 2);
}

TEST(SharedPtrResetRaw, AddsConstQualification) {
  SharedPtr<const int> pointer;
  pointer.reset(new int(37));
  EXPECT_EQ(*pointer, 37);
}

TEST(SharedPtrResetDeleter, UsesProvidedDeleter) {
  int calls = 0;
  {
    SharedPtr<int> pointer;
    pointer.reset(new int(3), CountingDeleter{&calls});
  }
  EXPECT_EQ(calls, 1);
}

TEST(SharedPtrResetDeleter, ReplacesPreviousDeleter) {
  int old_calls = 0;
  int new_calls = 0;
  {
    SharedPtr<int> pointer(new int(1), CountingDeleter{&old_calls});
    pointer.reset(new int(2), CountingDeleter{&new_calls});
    EXPECT_EQ(old_calls, 1);
    EXPECT_EQ(new_calls, 0);
  }
  EXPECT_EQ(new_calls, 1);
}

TEST(SharedPtrResetDeleter, PreservesValue) {
  int calls = 0;
  SharedPtr<int> pointer;
  pointer.reset(new int(19), CountingDeleter{&calls});
  EXPECT_EQ(*pointer, 19);
}

TEST(SharedPtrSwapMember, ExchangesPointers) {
  SharedPtr<int> first(new int(1));
  SharedPtr<int> second(new int(2));
  int* first_raw = first.get();
  int* second_raw = second.get();
  first.swap(second);
  EXPECT_EQ(first.get(), second_raw);
  EXPECT_EQ(second.get(), first_raw);
}

TEST(SharedPtrSwapMember, ExchangesOwnerGroups) {
  SharedPtr<int> first(new int(1));
  SharedPtr<int> first_copy(first);
  SharedPtr<int> second(new int(2));
  first.swap(second);
  EXPECT_TRUE(SharesOwnership(second, first_copy));
  EXPECT_TRUE(DifferentOwnership(first, first_copy));
}

TEST(SharedPtrSwapMember, HandlesEmptyAndNonEmpty) {
  SharedPtr<int> empty;
  SharedPtr<int> full(new int(8));
  empty.swap(full);
  EXPECT_EQ(*empty, 8);
  ExpectEmpty(full);
}

TEST(SharedPtrSwapMember, HandlesTwoEmptyPointers) {
  SharedPtr<int> first;
  SharedPtr<int> second;
  first.swap(second);
  ExpectEmpty(first);
  ExpectEmpty(second);
}

TEST(SharedPtrSwapMember, HandlesSelfSwap) {
  SharedPtr<int> pointer(new int(8));
  pointer.swap(pointer);
  EXPECT_EQ(*pointer, 8);
  EXPECT_EQ(pointer.use_count(), 1);
}

TEST(SharedPtrSwapMember, IsNoexcept) {
  static_assert(noexcept(std::declval<SharedPtr<int>&>().swap(std::declval<SharedPtr<int>&>())));
}

TEST(SharedPtrSwapNonMember, ExchangesPointers) {
  SharedPtr<int> first(new int(1));
  SharedPtr<int> second(new int(2));
  using std::swap;
  swap(first, second);
  EXPECT_EQ(*first, 2);
  EXPECT_EQ(*second, 1);
}

TEST(SharedPtrSwapNonMember, HandlesEmptyPointer) {
  SharedPtr<int> first;
  SharedPtr<int> second(new int(2));
  using std::swap;
  swap(first, second);
  EXPECT_EQ(*first, 2);
  ExpectEmpty(second);
}

TEST(SharedPtrSwapNonMember, DoesNotChangeUseCounts) {
  SharedPtr<int> first(new int(1));
  SharedPtr<int> first_copy(first);
  SharedPtr<int> second(new int(2));
  using std::swap;
  swap(first, second);
  EXPECT_EQ(second.use_count(), 2);
  EXPECT_EQ(first.use_count(), 1);
}

TEST(SharedPtrSwapNonMember, IsNoexcept) {
  static_assert(noexcept(std::swap(std::declval<SharedPtr<int>&>(), std::declval<SharedPtr<int>&>())));
}

TEST(SharedPtrVectorInteraction, SupportsRelocation) {
  SharedPtr<int> owner(new int(9));
  std::vector<SharedPtr<int>> pointers;
  for (int i = 0; i < 64; ++i) {
    pointers.push_back(owner);
  }
  EXPECT_EQ(owner.use_count(), 65);
  EXPECT_TRUE(std::all_of(pointers.begin(), pointers.end(), [&](const auto& p) {
    return p.get() == owner.get();
  }));
}

TEST(SharedPtrVectorInteraction, EraseDecrementsCount) {
  SharedPtr<int> owner(new int(9));
  std::vector<SharedPtr<int>> pointers(10, owner);
  pointers.erase(pointers.begin(), pointers.begin() + 4);
  EXPECT_EQ(owner.use_count(), 7);
}

TEST(SharedPtrVectorInteraction, ClearReleasesCopies) {
  SharedPtr<int> owner(new int(9));
  std::vector<SharedPtr<int>> pointers(10, owner);
  pointers.clear();
  EXPECT_EQ(owner.use_count(), 1);
}

TEST(SharedPtrVectorInteraction, MoveIntoContainerTransfersOwner) {
  SharedPtr<int> owner(new int(9));
  std::vector<SharedPtr<int>> pointers;
  pointers.push_back(std::move(owner));
  ExpectEmpty(owner);
  EXPECT_EQ(pointers.front().use_count(), 1);
}

TEST(SharedPtrModifierSequence, ResetSwapAndCopyStayConsistent) {
  SharedPtr<int> a(new int(1));
  SharedPtr<int> b(a);
  SharedPtr<int> c(new int(3));
  b.swap(c);
  EXPECT_EQ(*a, 1);
  EXPECT_EQ(*b, 3);
  EXPECT_EQ(*c, 1);
  c.reset();
  EXPECT_EQ(a.use_count(), 1);
}

}  // namespace shared_ptr_tests
