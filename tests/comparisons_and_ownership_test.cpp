#include "test_support.hpp"

#include "shared_ptr/weak_ptr.hpp"

namespace shared_ptr_tests {

TEST(SharedPtrEquality, CopiesCompareEqual) {
  SharedPtr<int> first(new int(1));
  SharedPtr<int> second(first);
  EXPECT_TRUE(first == second);
  EXPECT_FALSE(first != second);
}

TEST(SharedPtrEquality, IndependentObjectsCompareUnequal) {
  SharedPtr<int> first(new int(1));
  SharedPtr<int> second(new int(1));
  EXPECT_FALSE(first == second);
  EXPECT_TRUE(first != second);
}

TEST(SharedPtrEquality, TwoEmptyPointersCompareEqual) {
  SharedPtr<int> first;
  SharedPtr<int> second;
  EXPECT_TRUE(first == second);
}

TEST(SharedPtrEquality, EmptyAndNonEmptyCompareUnequal) {
  SharedPtr<int> empty;
  SharedPtr<int> full(new int);
  EXPECT_NE(empty, full);
}

TEST(SharedPtrEquality, ComparesStoredNotOwnedPointer) {
  struct Pair { int a; int b; };
  SharedPtr<Pair> owner(new Pair{1, 2});
  SharedPtr<int> first(owner, &owner->a);
  SharedPtr<int> second(owner, &owner->b);
  EXPECT_NE(first, second);
  EXPECT_TRUE(SharesOwnership(first, second));
}

TEST(SharedPtrEquality, NullAliasEqualsNullptrDespiteOwning) {
  SharedPtr<int> owner(new int);
  SharedPtr<int> null_alias(owner, nullptr);
  EXPECT_EQ(null_alias, nullptr);
  EXPECT_EQ(nullptr, null_alias);
  EXPECT_EQ(null_alias.use_count(), 2);
}

TEST(SharedPtrNullComparison, EmptyEqualsNullptr) {
  SharedPtr<int> pointer;
  EXPECT_TRUE(pointer == nullptr);
  EXPECT_TRUE(nullptr == pointer);
}

TEST(SharedPtrNullComparison, NonEmptyDoesNotEqualNullptr) {
  SharedPtr<int> pointer(new int);
  EXPECT_TRUE(pointer != nullptr);
  EXPECT_TRUE(nullptr != pointer);
}

TEST(SharedPtrMixedEquality, DerivedAndBaseCopiesCompareEqual) {
  SharedPtr<Derived> derived(new Derived);
  SharedPtr<Base> base(derived);
  EXPECT_EQ(derived, base);
  EXPECT_EQ(base, derived);
}

TEST(SharedPtrRelational, OrderingMatchesRawPointers) {
  SharedPtr<int> first(new int);
  SharedPtr<int> second(new int);
  const bool raw_less = std::less<int*>{}(first.get(), second.get());
  EXPECT_EQ(first < second, raw_less);
  EXPECT_EQ(first > second, second < first);
}

TEST(SharedPtrRelational, EqualPointersAreNeitherLessNorGreater) {
  SharedPtr<int> first(new int);
  SharedPtr<int> second(first);
  EXPECT_FALSE(first < second);
  EXPECT_FALSE(first > second);
  EXPECT_TRUE(first <= second);
  EXPECT_TRUE(first >= second);
}

TEST(SharedPtrRelational, OrderingIsAsymmetric) {
  SharedPtr<int> first(new int);
  SharedPtr<int> second(new int);
  EXPECT_FALSE((first < second) && (second < first));
}

TEST(SharedPtrOwnerBefore, CopiesAreEquivalent) {
  SharedPtr<int> first(new int);
  SharedPtr<int> second(first);
  EXPECT_FALSE(first.owner_before(second));
  EXPECT_FALSE(second.owner_before(first));
}

TEST(SharedPtrOwnerBefore, AliasesAreEquivalent) {
  struct Pair { int a; int b; };
  SharedPtr<Pair> owner(new Pair{1, 2});
  SharedPtr<int> first(owner, &owner->a);
  SharedPtr<int> second(owner, &owner->b);
  EXPECT_FALSE(first.owner_before(second));
  EXPECT_FALSE(second.owner_before(first));
}

TEST(SharedPtrOwnerBefore, IndependentOwnersAreStrictlyOrdered) {
  SharedPtr<int> first(new int);
  SharedPtr<int> second(new int);
  EXPECT_NE(first.owner_before(second), second.owner_before(first));
}

TEST(SharedPtrOwnerBefore, IsIrreflexive) {
  SharedPtr<int> pointer(new int);
  EXPECT_FALSE(pointer.owner_before(pointer));
}

TEST(SharedPtrOwnerBefore, EmptyPointersAreEquivalent) {
  SharedPtr<int> first;
  SharedPtr<double> second;
  EXPECT_FALSE(first.owner_before(second));
  EXPECT_FALSE(second.owner_before(first));
}

TEST(SharedPtrOwnerBefore, OwnedNullDiffersFromEmpty) {
  int calls = 0;
  SharedPtr<int> owned_null(nullptr, CountingDeleter{&calls});
  SharedPtr<int> empty;
  EXPECT_NE(owned_null.owner_before(empty), empty.owner_before(owned_null));
}

TEST(SharedPtrOwnerBefore, SupportsDifferentElementTypes) {
  SharedPtr<Derived> derived(new Derived);
  SharedPtr<Base> base(derived);
  EXPECT_FALSE(derived.owner_before(base));
  EXPECT_FALSE(base.owner_before(derived));
}

TEST(SharedPtrOwnerBefore, SupportsWeakPtrArgument) {
  SharedPtr<int> shared(new int);
  WeakPtr<int> weak(shared);
  EXPECT_FALSE(shared.owner_before(weak));
  EXPECT_FALSE(weak.owner_before(shared));
}

TEST(SharedPtrOwnerOrdering, CanGroupAliasesInOwnerSet) {
  struct OwnerLess {
    bool operator()(const SharedPtr<int>& a, const SharedPtr<int>& b) const {
      return a.owner_before(b);
    }
  };
  struct Pair { int a; int b; };
  SharedPtr<Pair> owner(new Pair{1, 2});
  SharedPtr<int> first(owner, &owner->a);
  SharedPtr<int> second(owner, &owner->b);
  std::set<SharedPtr<int>, OwnerLess> set;
  set.insert(first);
  set.insert(second);
  EXPECT_EQ(set.size(), 1u);
}

TEST(SharedPtrHash, CopiesHaveSameHash) {
  SharedPtr<int> first(new int);
  SharedPtr<int> second(first);
  EXPECT_EQ(std::hash<SharedPtr<int>>{}(first), std::hash<SharedPtr<int>>{}(second));
}

TEST(SharedPtrHash, HashMatchesStoredPointerHash) {
  SharedPtr<int> pointer(new int);
  EXPECT_EQ(std::hash<SharedPtr<int>>{}(pointer), std::hash<int*>{}(pointer.get()));
}

TEST(SharedPtrHash, EmptyPointerHashMatchesNullPointer) {
  SharedPtr<int> pointer;
  EXPECT_EQ(std::hash<SharedPtr<int>>{}(pointer), std::hash<int*>{}(nullptr));
}

TEST(SharedPtrStreamInsertion, WritesStoredPointerRepresentation) {
  SharedPtr<int> pointer(new int);
  std::ostringstream actual;
  std::ostringstream expected;
  actual << pointer;
  expected << pointer.get();
  EXPECT_EQ(actual.str(), expected.str());
}

TEST(SharedPtrStreamInsertion, WritesNullPointerRepresentation) {
  SharedPtr<int> pointer;
  std::ostringstream actual;
  std::ostringstream expected;
  actual << pointer;
  expected << pointer.get();
  EXPECT_EQ(actual.str(), expected.str());
}

}  // namespace shared_ptr_tests
