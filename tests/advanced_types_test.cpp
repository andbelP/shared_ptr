#include "test_support.hpp"

namespace shared_ptr_tests {

struct Incomplete;

TEST(SharedPtrIncompleteType, DefaultConstructionIsSupported) {
  SharedPtr<Incomplete> pointer;
  EXPECT_EQ(pointer.get(), nullptr);
  EXPECT_EQ(pointer.use_count(), 0);
}

TEST(SharedPtrIncompleteType, EmptyCopyIsSupported) {
  SharedPtr<Incomplete> first;
  SharedPtr<Incomplete> second(first);
  ExpectEmpty(second);
}

TEST(SharedPtrIncompleteType, EmptyMoveIsSupported) {
  SharedPtr<Incomplete> first;
  SharedPtr<Incomplete> second(std::move(first));
  ExpectEmpty(first);
  ExpectEmpty(second);
}

TEST(SharedPtrVoid, ConvertsFromObjectPointer) {
  SharedPtr<int> integer(new int(42));
  SharedPtr<void> erased(integer);
  EXPECT_EQ(erased.get(), static_cast<void*>(integer.get()));
  EXPECT_EQ(erased.use_count(), 2);
}

TEST(SharedPtrVoid, RawConstructionRetainsOriginalDeletionType) {
  LifetimeStats stats;
  { SharedPtr<void> erased(new Tracked(&stats)); }
  EXPECT_EQ(stats.destroyed, 1);
}

TEST(SharedPtrVoid, ConvertsToConstVoid) {
  SharedPtr<int> integer(new int(42));
  SharedPtr<const void> erased(integer);
  EXPECT_EQ(erased.get(), static_cast<const void*>(integer.get()));
}

TEST(SharedPtrVoid, ElementTypeIsVoid) {
  static_assert(std::is_same_v<SharedPtr<void>::element_type, void>);
  static_assert(std::is_same_v<SharedPtr<const void>::element_type, const void>);
}

inline int function_call_count = 0;
inline void TestFunction() { ++function_call_count; }

TEST(SharedPtrFunction, CanStoreFunctionPointerWithNoOpDeleter) {
  function_call_count = 0;
  SharedPtr<void()> function(&TestFunction, [](void (*)()) {});
  ASSERT_TRUE(function);
  (*function)();
  EXPECT_EQ(function_call_count, 1);
}

TEST(SharedPtrFunction, GetReturnsFunctionPointer) {
  SharedPtr<void()> function(&TestFunction, [](void (*)()) {});
  static_assert(std::is_same_v<decltype(function.get()), void (*)()>);
  EXPECT_EQ(function.get(), &TestFunction);
}

TEST(SharedPtrNonCopyableElement, PointerItselfRemainsCopyable) {
  struct NonCopyable {
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
    int value = 12;
  };
  SharedPtr<NonCopyable> first(new NonCopyable);
  SharedPtr<NonCopyable> second(first);
  EXPECT_EQ(second->value, 12);
  EXPECT_EQ(first.use_count(), 2);
}

TEST(SharedPtrOverAlignedElement, PreservesAlignment) {
  struct alignas(128) OverAligned { int value = 3; };
  SharedPtr<OverAligned> pointer(new OverAligned);
  const auto address = reinterpret_cast<std::uintptr_t>(pointer.get());
  EXPECT_EQ(address % alignof(OverAligned), 0u);
}

TEST(SharedPtrPointerElement, ManagesPointerObjectNotPointee) {
  int value = 8;
  SharedPtr<int*> pointer(new int*(&value));
  EXPECT_EQ(**pointer, 8);
  **pointer = 9;
  EXPECT_EQ(value, 9);
}

TEST(SharedPtrEnumElement, SupportsEnumerationTypes) {
  enum class State { idle, running };
  SharedPtr<State> pointer(new State(State::running));
  EXPECT_EQ(*pointer, State::running);
}

TEST(SharedPtrNoexceptContract, DestructorIsNoexcept) {
  static_assert(std::is_nothrow_destructible_v<SharedPtr<int>>);
}

TEST(SharedPtrNoexceptContract, ObserversAreNoexcept) {
  static_assert(noexcept(std::declval<const SharedPtr<int>&>().get()));
  static_assert(noexcept(std::declval<const SharedPtr<int>&>().use_count()));
  static_assert(noexcept(static_cast<bool>(std::declval<const SharedPtr<int>&>())));
}

TEST(SharedPtrNoexceptContract, OwnerBeforeIsNoexcept) {
  static_assert(noexcept(std::declval<const SharedPtr<int>&>().owner_before(
      std::declval<const SharedPtr<double>&>())));
}

TEST(SharedPtrTypeTraits, CopyOperationsDoNotRequireCopyableElement) {
  struct NonCopyable {
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
  };
  static_assert(std::is_copy_constructible_v<SharedPtr<NonCopyable>>);
  static_assert(std::is_copy_assignable_v<SharedPtr<NonCopyable>>);
}

TEST(SharedPtrArrayConversionRules, DerivedArrayDoesNotConvertToBaseArray) {
  static_assert(!std::is_constructible_v<SharedPtr<Base[]>,
                                         const SharedPtr<Derived[]>&>);
}

}  // namespace shared_ptr_tests
