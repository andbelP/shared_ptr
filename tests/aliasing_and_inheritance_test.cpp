#include "test_support.hpp"

namespace shared_ptr_tests {

TEST(SharedPtrAliasing, StoresMemberAddress) {
  struct Pair { int first; int second; };
  SharedPtr<Pair> owner(new Pair{10, 20});
  SharedPtr<int> alias(owner, &owner->second);
  EXPECT_EQ(alias.get(), &owner->second);
  EXPECT_EQ(*alias, 20);
}

TEST(SharedPtrAliasing, SharesOwnerGroup) {
  struct Pair { int first; int second; };
  SharedPtr<Pair> owner(new Pair{10, 20});
  SharedPtr<int> alias(owner, &owner->second);
  EXPECT_TRUE(SharesOwnership(owner, alias));
  EXPECT_EQ(owner.use_count(), 2);
}

TEST(SharedPtrAliasing, MutationAffectsCompleteObject) {
  struct Pair { int first; int second; };
  SharedPtr<Pair> owner(new Pair{10, 20});
  SharedPtr<int> alias(owner, &owner->second);
  *alias = 90;
  EXPECT_EQ(owner->second, 90);
}

TEST(SharedPtrAliasing, KeepsCompleteObjectAlive) {
  LifetimeStats stats;
  SharedPtr<Tracked> owner(new Tracked(&stats, 12));
  SharedPtr<int> alias(owner, &owner->value);
  owner.reset();
  EXPECT_EQ(stats.destroyed, 0);
  EXPECT_EQ(*alias, 12);
  alias.reset();
  EXPECT_EQ(stats.destroyed, 1);
}

TEST(SharedPtrAliasing, CanStoreNullWhileOwningObject) {
  SharedPtr<int> owner(new int(5));
  SharedPtr<double> alias(owner, nullptr);
  EXPECT_FALSE(alias);
  EXPECT_EQ(alias.get(), nullptr);
  EXPECT_EQ(alias.use_count(), 2);
  EXPECT_TRUE(SharesOwnership(owner, alias));
}

TEST(SharedPtrAliasing, EmptyOwnerWithNonNullStoredPointerHasNoOwnership) {
  int value = 7;
  SharedPtr<int> empty;
  SharedPtr<int> alias(empty, &value);
  EXPECT_TRUE(alias);
  EXPECT_EQ(alias.get(), &value);
  EXPECT_EQ(alias.use_count(), 0);
}

TEST(SharedPtrAliasing, CopyOfAliasPreservesStoredPointer) {
  struct Pair { int first; int second; };
  SharedPtr<Pair> owner(new Pair{1, 2});
  SharedPtr<int> alias(owner, &owner->second);
  SharedPtr<int> copy(alias);
  EXPECT_EQ(copy.get(), alias.get());
  EXPECT_EQ(copy.use_count(), 3);
}

TEST(SharedPtrAliasing, MoveOfAliasPreservesStoredPointer) {
  struct Pair { int first; int second; };
  SharedPtr<Pair> owner(new Pair{1, 2});
  SharedPtr<int> alias(owner, &owner->second);
  int* raw = alias.get();
  SharedPtr<int> moved(std::move(alias));
  ExpectEmpty(alias);
  EXPECT_EQ(moved.get(), raw);
}

TEST(SharedPtrAliasing, RvalueOwnerTransfersOwnershipToAlias) {
  struct Pair { int first; int second; };
  SharedPtr<Pair> owner(new Pair{1, 2});
  int* member = &owner->second;
  SharedPtr<int> alias(std::move(owner), member);
  ExpectEmpty(owner);
  EXPECT_EQ(alias.get(), member);
  EXPECT_EQ(alias.use_count(), 1);
}

TEST(SharedPtrAliasing, RvalueEmptyOwnerCanStoreNonNullPointer) {
  int value = 44;
  SharedPtr<int> empty;
  SharedPtr<int> alias(std::move(empty), &value);
  EXPECT_EQ(alias.get(), &value);
  EXPECT_EQ(alias.use_count(), 0);
}

TEST(SharedPtrAliasing, TwoMemberAliasesShareOwnership) {
  struct Pair { int first; int second; };
  SharedPtr<Pair> owner(new Pair{1, 2});
  SharedPtr<int> first(owner, &owner->first);
  SharedPtr<int> second(owner, &owner->second);
  EXPECT_NE(first.get(), second.get());
  EXPECT_TRUE(SharesOwnership(first, second));
  EXPECT_EQ(owner.use_count(), 3);
}

TEST(SharedPtrAliasing, BaseSubobjectSharesCompleteObject) {
  SharedPtr<MultiplyInherited> owner(new MultiplyInherited);
  SharedPtr<Right> right(owner, static_cast<Right*>(owner.get()));
  EXPECT_EQ(right->right, 22);
  EXPECT_TRUE(SharesOwnership(owner, right));
}

TEST(SharedPtrConvertingCopy, DerivedToBaseAdjustsPointer) {
  SharedPtr<MultiplyInherited> owner(new MultiplyInherited);
  SharedPtr<Right> right(owner);
  EXPECT_EQ(right.get(), static_cast<Right*>(owner.get()));
  EXPECT_EQ(right->right, 22);
}

TEST(SharedPtrConvertingCopy, DerivedToLeftSharesOwnership) {
  SharedPtr<MultiplyInherited> owner(new MultiplyInherited);
  SharedPtr<Left> left(owner);
  EXPECT_TRUE(SharesOwnership(owner, left));
  EXPECT_EQ(left.use_count(), 2);
}

TEST(SharedPtrConvertingCopy, DerivedToRightSharesOwnership) {
  SharedPtr<MultiplyInherited> owner(new MultiplyInherited);
  SharedPtr<Right> right(owner);
  EXPECT_TRUE(SharesOwnership(owner, right));
  EXPECT_EQ(right.use_count(), 2);
}

TEST(SharedPtrConvertingCopy, AddsTopLevelConstToElement) {
  SharedPtr<Derived> source(new Derived);
  SharedPtr<const Derived> target(source);
  EXPECT_EQ(target.get(), source.get());
}

TEST(SharedPtrConvertingCopy, DerivedToConstBase) {
  SharedPtr<Derived> source(new Derived);
  SharedPtr<const Base> target(source);
  EXPECT_EQ(target->Kind(), 2);
  EXPECT_EQ(target.use_count(), 2);
}

TEST(SharedPtrConvertingMove, DerivedToBaseEmptiesSource) {
  SharedPtr<Derived> source(new Derived);
  SharedPtr<Base> target(std::move(source));
  ExpectEmpty(source);
  EXPECT_EQ(target->Kind(), 2);
}

TEST(SharedPtrConvertingMove, MultipleInheritanceAdjustsPointer) {
  SharedPtr<MultiplyInherited> source(new MultiplyInherited);
  auto* expected = static_cast<Right*>(source.get());
  SharedPtr<Right> target(std::move(source));
  ExpectEmpty(source);
  EXPECT_EQ(target.get(), expected);
}

TEST(SharedPtrPolymorphism, VirtualDispatchWorks) {
  SharedPtr<Base> pointer(new Derived);
  EXPECT_EQ(pointer->Kind(), 2);
}

TEST(SharedPtrPolymorphism, CopyKeepsVirtualDispatch) {
  SharedPtr<Derived> derived(new Derived);
  SharedPtr<Base> base(derived);
  EXPECT_EQ(base->Kind(), 2);
}

TEST(SharedPtrPolymorphism, AbstractBaseIsSupported) {
  SharedPtr<Abstract> pointer(new Concrete(101));
  EXPECT_EQ(pointer->Value(), 101);
}

TEST(SharedPtrPolymorphism, ConvertingAssignmentSupportsAbstractBase) {
  SharedPtr<Concrete> concrete(new Concrete(25));
  SharedPtr<Abstract> abstract;
  abstract = concrete;
  EXPECT_EQ(abstract->Value(), 25);
}

TEST(SharedPtrConversionTraits, DerivedConvertsToBase) {
  static_assert(std::is_constructible_v<SharedPtr<Base>, const SharedPtr<Derived>&>);
}

TEST(SharedPtrConversionTraits, BaseDoesNotImplicitlyConvertToDerived) {
  static_assert(!std::is_constructible_v<SharedPtr<Derived>, const SharedPtr<Base>&>);
}

TEST(SharedPtrConversionTraits, MutableConvertsToConst) {
  static_assert(std::is_constructible_v<SharedPtr<const int>, const SharedPtr<int>&>);
}

TEST(SharedPtrConversionTraits, ConstDoesNotConvertToMutable) {
  static_assert(!std::is_constructible_v<SharedPtr<int>, const SharedPtr<const int>&>);
}

TEST(SharedPtrConversionTraits, UnrelatedTypesDoNotConvert) {
  static_assert(!std::is_constructible_v<SharedPtr<int>, const SharedPtr<double>&>);
}

}  // namespace shared_ptr_tests
