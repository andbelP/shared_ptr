#include "test_support.hpp"

namespace shared_ptr_tests {

TEST(SharedPtrArrayConstructor, StoresArrayAddress) {
  int* raw = new int[3]{1, 2, 3};
  SharedPtr<int[]> pointer(raw);
  EXPECT_EQ(pointer.get(), raw);
}

TEST(SharedPtrArrayConstructor, StartsWithOneOwner) {
  SharedPtr<int[]> pointer(new int[3]);
  EXPECT_EQ(pointer.use_count(), 1);
}

TEST(SharedPtrArrayConstructor, ConvertsToTrue) {
  SharedPtr<int[]> pointer(new int[3]);
  EXPECT_TRUE(pointer);
}

TEST(SharedPtrArraySubscript, ReadsFirstElement) {
  SharedPtr<int[]> pointer(new int[3]{4, 5, 6});
  EXPECT_EQ(pointer[0], 4);
}

TEST(SharedPtrArraySubscript, ReadsMiddleElement) {
  SharedPtr<int[]> pointer(new int[3]{4, 5, 6});
  EXPECT_EQ(pointer[1], 5);
}

TEST(SharedPtrArraySubscript, ReadsLastElement) {
  SharedPtr<int[]> pointer(new int[3]{4, 5, 6});
  EXPECT_EQ(pointer[2], 6);
}

TEST(SharedPtrArraySubscript, ReturnsLvalueReference) {
  static_assert(std::is_same_v<decltype(std::declval<const SharedPtr<int[]>&>()[0]), int&>);
}

TEST(SharedPtrArraySubscript, AllowsMutation) {
  SharedPtr<int[]> pointer(new int[3]{4, 5, 6});
  pointer[1] = 99;
  EXPECT_EQ(pointer[1], 99);
}

TEST(SharedPtrArraySubscript, MutationVisibleThroughCopy) {
  SharedPtr<int[]> first(new int[3]{4, 5, 6});
  SharedPtr<int[]> second(first);
  second[2] = 88;
  EXPECT_EQ(first[2], 88);
}

TEST(SharedPtrArrayCopy, SharesArrayAddress) {
  SharedPtr<int[]> first(new int[3]{1, 2, 3});
  SharedPtr<int[]> second(first);
  EXPECT_EQ(first.get(), second.get());
  EXPECT_EQ(first.use_count(), 2);
}

TEST(SharedPtrArrayMove, TransfersArrayAddress) {
  SharedPtr<int[]> source(new int[3]{1, 2, 3});
  int* raw = source.get();
  SharedPtr<int[]> destination(std::move(source));
  ExpectEmpty(source);
  EXPECT_EQ(destination.get(), raw);
}

TEST(SharedPtrArrayAssignment, CopySharesOwnership) {
  SharedPtr<int[]> source(new int[2]{3, 4});
  SharedPtr<int[]> destination;
  destination = source;
  EXPECT_EQ(destination.use_count(), 2);
  EXPECT_EQ(destination[1], 4);
}

TEST(SharedPtrArrayAssignment, MoveTransfersOwnership) {
  SharedPtr<int[]> source(new int[2]{3, 4});
  SharedPtr<int[]> destination;
  destination = std::move(source);
  ExpectEmpty(source);
  EXPECT_EQ(destination.use_count(), 1);
}

TEST(SharedPtrArrayReset, ReleasesArray) {
  int calls = 0;
  SharedPtr<int[]> pointer(new int[2], ArrayCountingDeleter{&calls});
  pointer.reset();
  EXPECT_EQ(calls, 1);
  ExpectEmpty(pointer);
}

TEST(SharedPtrArrayReset, ReplacesArray) {
  SharedPtr<int[]> pointer(new int[2]{1, 2});
  pointer.reset(new int[2]{3, 4});
  EXPECT_EQ(pointer.use_count(), 1);
  EXPECT_EQ(pointer[0], 3);
}

TEST(SharedPtrArrayReset, SupportsArrayDeleter) {
  int calls = 0;
  {
    SharedPtr<int[]> pointer;
    pointer.reset(new int[4], ArrayCountingDeleter{&calls});
  }
  EXPECT_EQ(calls, 1);
}

TEST(SharedPtrArrayDestruction, DestroysEveryElement) {
  struct CounterElement {
    explicit CounterElement(int* counters) : counters(counters) { ++counters[0]; }
    ~CounterElement() { --counters[0]; ++counters[1]; }
    int* counters;
  };
  int counters[2] = {0, 0};
  auto* raw = static_cast<CounterElement*>(::operator new[](3 * sizeof(CounterElement)));
  for (int i = 0; i < 3; ++i) new (raw + i) CounterElement(counters);
  auto deleter = [](CounterElement* values) {
    for (int i = 2; i >= 0; --i) values[i].~CounterElement();
    ::operator delete[](values);
  };
  { SharedPtr<CounterElement[]> pointer(raw, deleter); }
  EXPECT_EQ(counters[0], 0);
  EXPECT_EQ(counters[1], 3);
}

TEST(SharedPtrArrayCustomDeleter, CalledOnceAcrossCopies) {
  int calls = 0;
  {
    SharedPtr<int[]> first(new int[3], ArrayCountingDeleter{&calls});
    SharedPtr<int[]> second(first);
    SharedPtr<int[]> third(second);
  }
  EXPECT_EQ(calls, 1);
}

TEST(SharedPtrArrayCustomDeleter, RetrievableWithGetDeleter) {
  int calls = 0;
  SharedPtr<int[]> pointer(new int[3], ArrayCountingDeleter{&calls});
  auto* deleter = get_deleter<ArrayCountingDeleter>(pointer);
  ASSERT_NE(deleter, nullptr);
  EXPECT_EQ(deleter->calls, &calls);
}

TEST(SharedPtrArrayConstConversion, MutableArrayConvertsToConstArray) {
  SharedPtr<int[]> source(new int[2]{7, 8});
  SharedPtr<const int[]> destination(source);
  EXPECT_EQ(destination[0], 7);
  EXPECT_EQ(destination.use_count(), 2);
}

TEST(SharedPtrArrayConstConversion, IsSupportedByTraits) {
  static_assert(std::is_constructible_v<SharedPtr<const int[]>, const SharedPtr<int[]>&>);
}

TEST(SharedPtrArrayInvalidConversion, ConstArrayDoesNotConvertToMutable) {
  static_assert(!std::is_constructible_v<SharedPtr<int[]>, const SharedPtr<const int[]>&>);
}

TEST(SharedPtrArrayInvalidConversion, ArrayDoesNotConvertToScalarPointer) {
  static_assert(!std::is_constructible_v<SharedPtr<int>, const SharedPtr<int[]>&>);
}

TEST(SharedPtrArrayInvalidConversion, ScalarDoesNotConvertToArrayPointer) {
  static_assert(!std::is_constructible_v<SharedPtr<int[]>, const SharedPtr<int>&>);
}

TEST(SharedPtrArrayAliasing, CanAliasSingleElement) {
  SharedPtr<int[]> array(new int[3]{1, 2, 3});
  SharedPtr<int> element(array, &array[1]);
  EXPECT_EQ(*element, 2);
  EXPECT_TRUE(SharesOwnership(array, element));
}

TEST(SharedPtrArraySwap, ExchangesArrays) {
  SharedPtr<int[]> first(new int[2]{1, 2});
  SharedPtr<int[]> second(new int[2]{3, 4});
  first.swap(second);
  EXPECT_EQ(first[0], 3);
  EXPECT_EQ(second[0], 1);
}

TEST(SharedPtrArrayElementType, IsUnboundedArrayElement) {
  static_assert(std::is_same_v<SharedPtr<int[]>::element_type, int>);
}

TEST(SharedPtrArrayGet, ReturnsElementPointer) {
  static_assert(std::is_same_v<decltype(std::declval<SharedPtr<int[]>>().get()), int*>);
}

TEST(SharedPtrArrayVector, StoresAndCopiesArrayOwners) {
  SharedPtr<int[]> owner(new int[2]{6, 7});
  std::vector<SharedPtr<int[]>> owners(10, owner);
  EXPECT_EQ(owner.use_count(), 11);
  EXPECT_EQ(owners.back()[1], 7);
}

}  // namespace shared_ptr_tests
