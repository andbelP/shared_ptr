#include "test_support.hpp"

namespace shared_ptr_tests {

TEST(SharedPtrGet, ReturnsNullForEmptyPointer) {
  const SharedPtr<int> pointer;
  EXPECT_EQ(pointer.get(), nullptr);
}

TEST(SharedPtrGet, ReturnsManagedAddress) {
  int* raw = new int(5);
  const SharedPtr<int> pointer(raw);
  EXPECT_EQ(pointer.get(), raw);
}

TEST(SharedPtrGet, IsCallableOnConstObject) {
  static_assert(std::is_same_v<decltype(std::declval<const SharedPtr<int>&>().get()), int*>);
}

TEST(SharedPtrDereference, ReturnsManagedValue) {
  SharedPtr<int> pointer(new int(31));
  EXPECT_EQ(*pointer, 31);
}

TEST(SharedPtrDereference, ReturnsLvalueReference) {
  static_assert(std::is_same_v<decltype(*std::declval<const SharedPtr<int>&>()), int&>);
}

TEST(SharedPtrDereference, AllowsMutation) {
  SharedPtr<int> pointer(new int(31));
  *pointer = 64;
  EXPECT_EQ(*pointer, 64);
}

TEST(SharedPtrDereference, MutationVisibleThroughCopies) {
  SharedPtr<int> first(new int(31));
  SharedPtr<int> second(first);
  *second = 64;
  EXPECT_EQ(*first, 64);
}

TEST(SharedPtrArrow, CallsMemberFunction) {
  SharedPtr<Tracked> pointer(new Tracked(nullptr, 21));
  EXPECT_EQ(pointer->Double(), 42);
}

TEST(SharedPtrArrow, AccessesDataMember) {
  SharedPtr<Tracked> pointer(new Tracked(nullptr, 21));
  EXPECT_EQ(pointer->value, 21);
}

TEST(SharedPtrArrow, AllowsMemberMutation) {
  SharedPtr<Tracked> pointer(new Tracked(nullptr, 21));
  pointer->value = 10;
  EXPECT_EQ(pointer->Double(), 20);
}

TEST(SharedPtrArrow, ReturnsElementPointer) {
  static_assert(std::is_same_v<decltype(std::declval<const SharedPtr<Tracked>&>().operator->()), Tracked*>);
}

TEST(SharedPtrBool, EmptyPointerIsFalse) {
  SharedPtr<int> pointer;
  EXPECT_FALSE(static_cast<bool>(pointer));
}

TEST(SharedPtrBool, NullptrConstructedPointerIsFalse) {
  SharedPtr<int> pointer(nullptr);
  EXPECT_FALSE(static_cast<bool>(pointer));
}

TEST(SharedPtrBool, NonNullPointerIsTrue) {
  SharedPtr<int> pointer(new int);
  EXPECT_TRUE(static_cast<bool>(pointer));
}

TEST(SharedPtrBool, IsExplicit) {
  static_assert(std::is_constructible_v<bool, SharedPtr<int>>);
  static_assert(!std::is_convertible_v<SharedPtr<int>, bool>);
}

TEST(SharedPtrBool, WorksInConditional) {
  SharedPtr<int> pointer(new int(5));
  int result = 0;
  if (pointer) {
    result = *pointer;
  }
  EXPECT_EQ(result, 5);
}

TEST(SharedPtrUseCount, EmptyPointerHasZeroOwners) {
  SharedPtr<int> pointer;
  EXPECT_EQ(pointer.use_count(), 0);
}

TEST(SharedPtrUseCount, NewObjectHasOneOwner) {
  SharedPtr<int> pointer(new int);
  EXPECT_EQ(pointer.use_count(), 1);
}

TEST(SharedPtrUseCount, CopyIncrementsAllViews) {
  SharedPtr<int> first(new int);
  SharedPtr<int> second(first);
  EXPECT_EQ(first.use_count(), 2);
  EXPECT_EQ(second.use_count(), 2);
}

TEST(SharedPtrUseCount, NestedCopyTracksCount) {
  SharedPtr<int> first(new int);
  EXPECT_EQ(first.use_count(), 1);
  {
    SharedPtr<int> second(first);
    EXPECT_EQ(first.use_count(), 2);
    {
      SharedPtr<int> third(second);
      EXPECT_EQ(first.use_count(), 3);
    }
    EXPECT_EQ(first.use_count(), 2);
  }
  EXPECT_EQ(first.use_count(), 1);
}

TEST(SharedPtrUseCount, MoveLeavesCountUnchanged) {
  SharedPtr<int> first(new int);
  SharedPtr<int> second(std::move(first));
  EXPECT_EQ(second.use_count(), 1);
  EXPECT_EQ(first.use_count(), 0);
}

TEST(SharedPtrUseCount, ResetCopyDecrementsCount) {
  SharedPtr<int> first(new int);
  SharedPtr<int> second(first);
  second.reset();
  EXPECT_EQ(first.use_count(), 1);
  EXPECT_EQ(second.use_count(), 0);
}

TEST(SharedPtrUseCount, AliasesShareCount) {
  struct Pair { int first; int second; };
  SharedPtr<Pair> owner(new Pair{1, 2});
  SharedPtr<int> alias(owner, &owner->second);
  EXPECT_EQ(owner.use_count(), 2);
  EXPECT_EQ(alias.use_count(), 2);
}

TEST(SharedPtrUseCount, ReturnTypeIsLong) {
  static_assert(std::is_same_v<decltype(std::declval<const SharedPtr<int>&>().use_count()), long>);
}

TEST(SharedPtrObserverConstness, ConstSmartPointerDoesNotConstQualifyElement) {
  const SharedPtr<int> pointer(new int(1));
  *pointer = 2;
  EXPECT_EQ(*pointer, 2);
}

TEST(SharedPtrObserverConstness, ConstElementCannotBeMutated) {
  static_assert(std::is_same_v<decltype(*std::declval<const SharedPtr<const int>&>()), const int&>);
}

TEST(SharedPtrObserverType, GetMatchesElementPointer) {
  static_assert(std::is_same_v<decltype(std::declval<SharedPtr<double>>().get()), double*>);
  static_assert(std::is_same_v<decltype(std::declval<SharedPtr<const double>>().get()), const double*>);
}

TEST(SharedPtrObserverType, DereferenceSupportsPolymorphism) {
  SharedPtr<Base> pointer(new Derived);
  EXPECT_EQ((*pointer).Kind(), 2);
}

TEST(SharedPtrObserverType, ArrowSupportsPolymorphism) {
  SharedPtr<Abstract> pointer(new Concrete(19));
  EXPECT_EQ(pointer->Value(), 19);
}

TEST(SharedPtrObserverType, SupportsIncompleteDeclaration) {
  struct CompleteHere { int value = 7; };
  SharedPtr<CompleteHere> pointer(new CompleteHere);
  EXPECT_EQ(pointer->value, 7);
}

}  // namespace shared_ptr_tests
