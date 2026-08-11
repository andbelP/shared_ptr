#include "test_support.hpp"

namespace shared_ptr_tests {

TEST(SharedPtrDefaultConstructor, CreatesEmptyPointer) {
  SharedPtr<int> pointer;
  ExpectEmpty(pointer);
}

TEST(SharedPtrDefaultConstructor, IsNoexcept) {
  static_assert(std::is_nothrow_default_constructible_v<SharedPtr<int>>);
}

TEST(SharedPtrDefaultConstructor, WorksForConstElement) {
  SharedPtr<const int> pointer;
  ExpectEmpty(pointer);
}

TEST(SharedPtrDefaultConstructor, WorksForVolatileElement) {
  SharedPtr<volatile int> pointer;
  EXPECT_EQ(pointer.get(), nullptr);
  EXPECT_EQ(pointer.use_count(), 0);
}

TEST(SharedPtrNullptrConstructor, CreatesEmptyPointer) {
  SharedPtr<int> pointer(nullptr);
  ExpectEmpty(pointer);
}

TEST(SharedPtrNullptrConstructor, IsNoexcept) {
  static_assert(std::is_nothrow_constructible_v<SharedPtr<int>, std::nullptr_t>);
}

TEST(SharedPtrRawPointerConstructor, StoresExactAddress) {
  auto* raw = new int(42);
  SharedPtr<int> pointer(raw);
  EXPECT_EQ(pointer.get(), raw);
}

TEST(SharedPtrRawPointerConstructor, StartsWithOneOwner) {
  SharedPtr<int> pointer(new int(42));
  EXPECT_EQ(pointer.use_count(), 1);
}

TEST(SharedPtrRawPointerConstructor, ConvertsToTrue) {
  SharedPtr<int> pointer(new int(42));
  EXPECT_TRUE(pointer);
}

TEST(SharedPtrRawPointerConstructor, PreservesValue) {
  SharedPtr<int> pointer(new int(42));
  EXPECT_EQ(*pointer, 42);
}

TEST(SharedPtrRawPointerConstructor, IsExplicit) {
  static_assert(std::is_constructible_v<SharedPtr<int>, int*>);
  static_assert(!std::is_convertible_v<int*, SharedPtr<int>>);
}

TEST(SharedPtrRawPointerConstructor, AcceptsDerivedForBase) {
  SharedPtr<Base> pointer(new Derived);
  EXPECT_EQ(pointer->Kind(), 2);
}

TEST(SharedPtrRawPointerConstructor, DeletesThroughOriginalPointerType) {
  struct NonVirtualBase {};
  struct DerivedWithDestructor : NonVirtualBase {
    explicit DerivedWithDestructor(bool* destroyed) : destroyed(destroyed) {}
    ~DerivedWithDestructor() { *destroyed = true; }
    bool* destroyed;
  };
  bool destroyed = false;
  { SharedPtr<NonVirtualBase> pointer(new DerivedWithDestructor(&destroyed)); }
  EXPECT_TRUE(destroyed);
}

TEST(SharedPtrRawPointerConstructor, SupportsConstQualification) {
  SharedPtr<const int> pointer(new int(17));
  EXPECT_EQ(*pointer, 17);
}

TEST(SharedPtrRawPointerConstructor, SupportsPolymorphicAbstractBase) {
  SharedPtr<Abstract> pointer(new Concrete(73));
  EXPECT_EQ(pointer->Value(), 73);
}

TEST(SharedPtrDeleterConstructor, InvokesCustomDeleter) {
  int calls = 0;
  { SharedPtr<int> pointer(new int(4), CountingDeleter{&calls}); }
  EXPECT_EQ(calls, 1);
}

TEST(SharedPtrDeleterConstructor, PreservesStoredValue) {
  int calls = 0;
  SharedPtr<int> pointer(new int(91), CountingDeleter{&calls});
  EXPECT_EQ(*pointer, 91);
}

TEST(SharedPtrDeleterConstructor, StartsWithOneOwner) {
  int calls = 0;
  SharedPtr<int> pointer(new int, CountingDeleter{&calls});
  EXPECT_EQ(pointer.use_count(), 1);
}

TEST(SharedPtrNullDeleterConstructor, OwnsNullPointer) {
  int calls = 0;
  bool received_null = false;
  SharedPtr<int> pointer(nullptr, NullAwareDeleter{&calls, &received_null});
  EXPECT_EQ(pointer.get(), nullptr);
  EXPECT_FALSE(pointer);
  EXPECT_EQ(pointer.use_count(), 1);
}

TEST(SharedPtrNullDeleterConstructor, CallsDeleterForNullPointer) {
  int calls = 0;
  bool received_null = false;
  {
    SharedPtr<int> pointer(nullptr, NullAwareDeleter{&calls, &received_null});
  }
  EXPECT_EQ(calls, 1);
  EXPECT_TRUE(received_null);
}

TEST(SharedPtrUniquePtrConstructor, TransfersOwnership) {
  auto source = std::make_unique<int>(55);
  int* raw = source.get();
  SharedPtr<int> pointer(std::move(source));
  EXPECT_EQ(source, nullptr);
  EXPECT_EQ(pointer.get(), raw);
  EXPECT_EQ(*pointer, 55);
}

TEST(SharedPtrUniquePtrConstructor, StartsWithOneOwner) {
  auto source = std::make_unique<int>(55);
  SharedPtr<int> pointer(std::move(source));
  EXPECT_EQ(pointer.use_count(), 1);
}

TEST(SharedPtrUniquePtrConstructor, ConvertsDerivedToBase) {
  auto source = std::make_unique<Derived>();
  SharedPtr<Base> pointer(std::move(source));
  EXPECT_EQ(pointer->Kind(), 2);
  EXPECT_EQ(source, nullptr);
}

TEST(SharedPtrUniquePtrConstructor, RetainsCustomDeleter) {
  int calls = 0;
  {
    std::unique_ptr<int, CountingDeleter> source(new int(8), CountingDeleter{&calls});
    SharedPtr<int> pointer(std::move(source));
  }
  EXPECT_EQ(calls, 1);
}

TEST(SharedPtrCopyConstructor, SharesSameObject) {
  SharedPtr<int> first(new int(8));
  SharedPtr<int> second(first);
  EXPECT_EQ(second.get(), first.get());
  EXPECT_TRUE(SharesOwnership(first, second));
}

TEST(SharedPtrCopyConstructor, IncrementsOwnerCount) {
  SharedPtr<int> first(new int(8));
  SharedPtr<int> second(first);
  EXPECT_EQ(first.use_count(), 2);
  EXPECT_EQ(second.use_count(), 2);
}

TEST(SharedPtrCopyConstructor, CopyingEmptyStaysEmpty) {
  SharedPtr<int> first;
  SharedPtr<int> second(first);
  ExpectEmpty(second);
}

TEST(SharedPtrConvertingCopyConstructor, ConvertsDerivedToBase) {
  SharedPtr<Derived> derived(new Derived);
  SharedPtr<Base> base(derived);
  EXPECT_EQ(base.get(), derived.get());
  EXPECT_EQ(base->Kind(), 2);
  EXPECT_EQ(base.use_count(), 2);
}

TEST(SharedPtrConvertingCopyConstructor, AddsConstQualification) {
  SharedPtr<int> mutable_pointer(new int(7));
  SharedPtr<const int> const_pointer(mutable_pointer);
  EXPECT_EQ(const_pointer.get(), mutable_pointer.get());
  EXPECT_EQ(const_pointer.use_count(), 2);
}

TEST(SharedPtrMoveConstructor, TransfersObject) {
  SharedPtr<int> source(new int(99));
  int* raw = source.get();
  SharedPtr<int> destination(std::move(source));
  ExpectEmpty(source);
  EXPECT_EQ(destination.get(), raw);
}

TEST(SharedPtrMoveConstructor, DoesNotIncreaseOwnerCount) {
  SharedPtr<int> source(new int(99));
  SharedPtr<int> destination(std::move(source));
  EXPECT_EQ(destination.use_count(), 1);
}

TEST(SharedPtrMoveConstructor, IsNoexcept) {
  static_assert(std::is_nothrow_move_constructible_v<SharedPtr<int>>);
}

TEST(SharedPtrConvertingMoveConstructor, ConvertsDerivedToBase) {
  SharedPtr<Derived> source(new Derived);
  Derived* raw = source.get();
  SharedPtr<Base> destination(std::move(source));
  ExpectEmpty(source);
  EXPECT_EQ(destination.get(), raw);
  EXPECT_EQ(destination.use_count(), 1);
}

TEST(SharedPtrTypeProperties, ElementTypeMatchesTemplateArgument) {
  static_assert(std::is_same_v<SharedPtr<int>::element_type, int>);
  static_assert(std::is_same_v<SharedPtr<const Derived>::element_type, const Derived>);
}

TEST(SharedPtrTypeProperties, IsCopyAndMoveConstructible) {
  static_assert(std::is_copy_constructible_v<SharedPtr<int>>);
  static_assert(std::is_move_constructible_v<SharedPtr<int>>);
}

}  // namespace shared_ptr_tests
