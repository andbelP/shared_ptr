#include "test_support.hpp"

namespace shared_ptr_tests {

TEST(SharedPtrCopyAssignment, SharesObject) {
  SharedPtr<int> source(new int(42));
  SharedPtr<int> target;
  target = source;
  EXPECT_EQ(target.get(), source.get());
}

TEST(SharedPtrCopyAssignment, IncrementsOwnerCount) {
  SharedPtr<int> source(new int(42));
  SharedPtr<int> target;
  target = source;
  EXPECT_EQ(source.use_count(), 2);
  EXPECT_EQ(target.use_count(), 2);
}

TEST(SharedPtrCopyAssignment, ReleasesPreviousObject) {
  LifetimeStats first_stats;
  LifetimeStats second_stats;
  SharedPtr<Tracked> target(new Tracked(&first_stats));
  SharedPtr<Tracked> source(new Tracked(&second_stats));
  target = source;
  EXPECT_EQ(first_stats.destroyed, 1);
  EXPECT_EQ(second_stats.destroyed, 0);
}

TEST(SharedPtrCopyAssignment, HandlesEmptySource) {
  SharedPtr<int> source;
  SharedPtr<int> target(new int(1));
  target = source;
  ExpectEmpty(target);
}

TEST(SharedPtrCopyAssignment, HandlesEmptyTarget) {
  SharedPtr<int> source(new int(1));
  SharedPtr<int> target;
  target = source;
  EXPECT_EQ(*target, 1);
  EXPECT_EQ(target.use_count(), 2);
}

TEST(SharedPtrCopyAssignment, HandlesBothEmpty) {
  SharedPtr<int> source;
  SharedPtr<int> target;
  target = source;
  ExpectEmpty(source);
  ExpectEmpty(target);
}

TEST(SharedPtrCopyAssignment, HandlesSelfAssignment) {
  SharedPtr<int> pointer(new int(9));
  auto* address = &pointer;
  pointer = *address;
  EXPECT_EQ(*pointer, 9);
  EXPECT_EQ(pointer.use_count(), 1);
}

TEST(SharedPtrCopyAssignment, SelfAssignmentDoesNotDestroyObject) {
  LifetimeStats stats;
  SharedPtr<Tracked> pointer(new Tracked(&stats));
  auto* address = &pointer;
  pointer = *address;
  EXPECT_EQ(stats.destroyed, 0);
}

TEST(SharedPtrCopyAssignment, ReturnsReferenceToTarget) {
  SharedPtr<int> source(new int(2));
  SharedPtr<int> target;
  static_assert(std::is_same_v<decltype(target = source), SharedPtr<int>&>);
  EXPECT_EQ(&(target = source), &target);
}

TEST(SharedPtrConvertingCopyAssignment, ConvertsDerivedToBase) {
  SharedPtr<Derived> source(new Derived);
  SharedPtr<Base> target;
  target = source;
  EXPECT_EQ(target->Kind(), 2);
  EXPECT_EQ(target.use_count(), 2);
}

TEST(SharedPtrConvertingCopyAssignment, AddsConstQualification) {
  SharedPtr<int> source(new int(12));
  SharedPtr<const int> target;
  target = source;
  EXPECT_EQ(*target, 12);
  EXPECT_EQ(target.use_count(), 2);
}

TEST(SharedPtrConvertingCopyAssignment, ReleasesOldBaseObject) {
  int calls = 0;
  SharedPtr<Base> target(new Base, CountingDeleter{&calls});
  SharedPtr<Derived> source(new Derived);
  target = source;
  EXPECT_EQ(calls, 1);
}

TEST(SharedPtrMoveAssignment, TransfersObject) {
  SharedPtr<int> source(new int(42));
  int* raw = source.get();
  SharedPtr<int> target;
  target = std::move(source);
  ExpectEmpty(source);
  EXPECT_EQ(target.get(), raw);
}

TEST(SharedPtrMoveAssignment, DoesNotIncreaseOwnerCount) {
  SharedPtr<int> source(new int(42));
  SharedPtr<int> target;
  target = std::move(source);
  EXPECT_EQ(target.use_count(), 1);
}

TEST(SharedPtrMoveAssignment, ReleasesPreviousObject) {
  LifetimeStats first_stats;
  LifetimeStats second_stats;
  SharedPtr<Tracked> target(new Tracked(&first_stats));
  SharedPtr<Tracked> source(new Tracked(&second_stats));
  target = std::move(source);
  EXPECT_EQ(first_stats.destroyed, 1);
  EXPECT_EQ(second_stats.destroyed, 0);
}

TEST(SharedPtrMoveAssignment, HandlesEmptySource) {
  SharedPtr<int> source;
  SharedPtr<int> target(new int(3));
  target = std::move(source);
  ExpectEmpty(target);
  ExpectEmpty(source);
}

TEST(SharedPtrMoveAssignment, HandlesEmptyTarget) {
  SharedPtr<int> source(new int(3));
  SharedPtr<int> target;
  target = std::move(source);
  EXPECT_EQ(*target, 3);
  ExpectEmpty(source);
}

TEST(SharedPtrMoveAssignment, HandlesBothEmpty) {
  SharedPtr<int> source;
  SharedPtr<int> target;
  target = std::move(source);
  ExpectEmpty(target);
}

TEST(SharedPtrMoveAssignment, HandlesSelfMoveAssignment) {
  SharedPtr<int> pointer(new int(27));
  auto* address = &pointer;
  pointer = std::move(*address);
  EXPECT_TRUE(pointer);
  EXPECT_EQ(*pointer, 27);
  EXPECT_EQ(pointer.use_count(), 1);
}

TEST(SharedPtrMoveAssignment, ReturnsReferenceToTarget) {
  SharedPtr<int> source(new int(2));
  SharedPtr<int> target;
  static_assert(std::is_same_v<decltype(target = std::move(source)), SharedPtr<int>&>);
}

TEST(SharedPtrMoveAssignment, IsNoexcept) {
  static_assert(std::is_nothrow_move_assignable_v<SharedPtr<int>>);
}

TEST(SharedPtrConvertingMoveAssignment, ConvertsDerivedToBase) {
  SharedPtr<Derived> source(new Derived);
  SharedPtr<Base> target;
  target = std::move(source);
  ExpectEmpty(source);
  EXPECT_EQ(target->Kind(), 2);
  EXPECT_EQ(target.use_count(), 1);
}

TEST(SharedPtrConvertingMoveAssignment, AddsConstQualification) {
  SharedPtr<int> source(new int(14));
  SharedPtr<const int> target;
  target = std::move(source);
  ExpectEmpty(source);
  EXPECT_EQ(*target, 14);
}

TEST(SharedPtrUniquePtrAssignment, TransfersOwnership) {
  auto source = std::make_unique<int>(71);
  int* raw = source.get();
  SharedPtr<int> target;
  target = std::move(source);
  EXPECT_EQ(source, nullptr);
  EXPECT_EQ(target.get(), raw);
}

TEST(SharedPtrUniquePtrAssignment, ReleasesPreviousObject) {
  int calls = 0;
  SharedPtr<int> target(new int(1), CountingDeleter{&calls});
  auto source = std::make_unique<int>(2);
  target = std::move(source);
  EXPECT_EQ(calls, 1);
  EXPECT_EQ(*target, 2);
}

TEST(SharedPtrUniquePtrAssignment, ConvertsDerivedToBase) {
  auto source = std::make_unique<Derived>();
  SharedPtr<Base> target;
  target = std::move(source);
  EXPECT_EQ(source, nullptr);
  EXPECT_EQ(target->Kind(), 2);
}

TEST(SharedPtrUniquePtrAssignment, PreservesCustomDeleter) {
  int calls = 0;
  {
    std::unique_ptr<int, CountingDeleter> source(new int(7), CountingDeleter{&calls});
    SharedPtr<int> target;
    target = std::move(source);
  }
  EXPECT_EQ(calls, 1);
}

TEST(SharedPtrAssignmentChain, SupportsCopyChaining) {
  SharedPtr<int> source(new int(88));
  SharedPtr<int> middle;
  SharedPtr<int> target;
  target = middle = source;
  EXPECT_EQ(source.use_count(), 3);
  EXPECT_EQ(target.get(), source.get());
}

TEST(SharedPtrAssignmentChain, SupportsMoveThenCopy) {
  SharedPtr<int> source(new int(88));
  SharedPtr<int> middle;
  SharedPtr<int> target;
  middle = std::move(source);
  target = middle;
  ExpectEmpty(source);
  EXPECT_EQ(middle.use_count(), 2);
  EXPECT_EQ(target.use_count(), 2);
}

TEST(SharedPtrAssignmentTypeProperties, IsCopyAssignable) {
  static_assert(std::is_copy_assignable_v<SharedPtr<int>>);
}

TEST(SharedPtrAssignmentTypeProperties, SupportsDerivedToBaseCopyAssignment) {
  static_assert(std::is_assignable_v<SharedPtr<Base>&, const SharedPtr<Derived>&>);
}

TEST(SharedPtrAssignmentTypeProperties, SupportsDerivedToBaseMoveAssignment) {
  static_assert(std::is_assignable_v<SharedPtr<Base>&, SharedPtr<Derived>&&>);
}

TEST(SharedPtrAssignmentTypeProperties, RejectsBaseToDerivedAssignment) {
  static_assert(!std::is_assignable_v<SharedPtr<Derived>&, const SharedPtr<Base>&>);
}

}  // namespace shared_ptr_tests
