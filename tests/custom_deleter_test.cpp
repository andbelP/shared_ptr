#include "test_support.hpp"

namespace shared_ptr_tests {

TEST(SharedPtrCustomDeleter, CalledExactlyOnceForSoleOwner) {
  int calls = 0;
  { SharedPtr<int> pointer(new int, CountingDeleter{&calls}); }
  EXPECT_EQ(calls, 1);
}

TEST(SharedPtrCustomDeleter, NotCalledWhenIntermediateCopyDies) {
  int calls = 0;
  SharedPtr<int> first(new int, CountingDeleter{&calls});
  { SharedPtr<int> second(first); }
  EXPECT_EQ(calls, 0);
}

TEST(SharedPtrCustomDeleter, CalledWhenLastOwnerResets) {
  int calls = 0;
  SharedPtr<int> first(new int, CountingDeleter{&calls});
  SharedPtr<int> second(first);
  first.reset();
  EXPECT_EQ(calls, 0);
  second.reset();
  EXPECT_EQ(calls, 1);
}

TEST(SharedPtrCustomDeleter, SurvivesCopyConstruction) {
  int calls = 0;
  {
    SharedPtr<int> first(new int, CountingDeleter{&calls});
    SharedPtr<int> second(first);
  }
  EXPECT_EQ(calls, 1);
}

TEST(SharedPtrCustomDeleter, SurvivesMoveConstruction) {
  int calls = 0;
  {
    SharedPtr<int> first(new int, CountingDeleter{&calls});
    SharedPtr<int> second(std::move(first));
  }
  EXPECT_EQ(calls, 1);
}

TEST(SharedPtrCustomDeleter, SurvivesCopyAssignment) {
  int calls = 0;
  {
    SharedPtr<int> first(new int, CountingDeleter{&calls});
    SharedPtr<int> second;
    second = first;
  }
  EXPECT_EQ(calls, 1);
}

TEST(SharedPtrCustomDeleter, SurvivesMoveAssignment) {
  int calls = 0;
  {
    SharedPtr<int> first(new int, CountingDeleter{&calls});
    SharedPtr<int> second;
    second = std::move(first);
  }
  EXPECT_EQ(calls, 1);
}

TEST(SharedPtrCustomDeleter, SupportsLambda) {
  int calls = 0;
  {
    SharedPtr<int> pointer(new int, [&](int* p) { ++calls; delete p; });
  }
  EXPECT_EQ(calls, 1);
}

TEST(SharedPtrCustomDeleter, SupportsFunctionPointer) {
  static int calls = 0;
  calls = 0;
  auto deleter = +[](int* p) { ++calls; delete p; };
  { SharedPtr<int> pointer(new int, deleter); }
  EXPECT_EQ(calls, 1);
}

TEST(SharedPtrCustomDeleter, SupportsMoveOnlyState) {
  struct MoveOnlyDeleter {
    explicit MoveOnlyDeleter(int* calls) : calls(calls) {}
    MoveOnlyDeleter(const MoveOnlyDeleter&) = delete;
    MoveOnlyDeleter(MoveOnlyDeleter&&) = default;
    void operator()(int* p) { ++*calls; delete p; }
    int* calls;
  };
  int calls = 0;
  { SharedPtr<int> pointer(new int, MoveOnlyDeleter(&calls)); }
  EXPECT_EQ(calls, 1);
}

TEST(SharedPtrCustomDeleter, CanImplementNoDeletePolicy) {
  int stack_value = 33;
  int calls = 0;
  {
    SharedPtr<int> pointer(&stack_value, [&](int*) { ++calls; });
    EXPECT_EQ(*pointer, 33);
  }
  EXPECT_EQ(calls, 1);
  EXPECT_EQ(stack_value, 33);
}

TEST(SharedPtrCustomDeleter, ReceivesExactPointer) {
  int* observed = nullptr;
  int* raw = new int(5);
  {
    SharedPtr<int> pointer(raw, [&](int* p) { observed = p; delete p; });
  }
  EXPECT_EQ(observed, raw);
}

TEST(SharedPtrCustomDeleter, DeleterCanMutateExternalState) {
  std::string event;
  {
    SharedPtr<int> pointer(new int, [&](int* p) { event = "deleted"; delete p; });
  }
  EXPECT_EQ(event, "deleted");
}

TEST(SharedPtrCustomDeleter, StateIsIndependentAcrossOwnerGroups) {
  int first_calls = 0;
  int second_calls = 0;
  {
    SharedPtr<int> first(new int, CountingDeleter{&first_calls});
    SharedPtr<int> second(new int, CountingDeleter{&second_calls});
    first.reset();
    EXPECT_EQ(first_calls, 1);
    EXPECT_EQ(second_calls, 0);
  }
  EXPECT_EQ(second_calls, 1);
}

TEST(SharedPtrGetDeleter, FindsExactDeleterType) {
  int calls = 0;
  SharedPtr<int> pointer(new int, StatefulDeleter{77, &calls});
  auto* deleter = get_deleter<StatefulDeleter>(pointer);
  ASSERT_NE(deleter, nullptr);
  EXPECT_EQ(deleter->id, 77);
  EXPECT_EQ(deleter->calls, &calls);
}

TEST(SharedPtrGetDeleter, ReturnsNullForWrongType) {
  int calls = 0;
  SharedPtr<int> pointer(new int, StatefulDeleter{77, &calls});
  EXPECT_EQ(get_deleter<CountingDeleter>(pointer), nullptr);
}

TEST(SharedPtrGetDeleter, ReturnsNullForDefaultDeleterStorage) {
  SharedPtr<int> pointer(new int);
  EXPECT_EQ(get_deleter<StatefulDeleter>(pointer), nullptr);
}

TEST(SharedPtrGetDeleter, ReturnsNullForEmptyPointer) {
  SharedPtr<int> pointer;
  EXPECT_EQ(get_deleter<StatefulDeleter>(pointer), nullptr);
}

TEST(SharedPtrGetDeleter, SameAddressThroughCopies) {
  int calls = 0;
  SharedPtr<int> first(new int, StatefulDeleter{7, &calls});
  SharedPtr<int> second(first);
  EXPECT_EQ(get_deleter<StatefulDeleter>(first), get_deleter<StatefulDeleter>(second));
}

TEST(SharedPtrGetDeleter, SameAddressThroughAliasingPointer) {
  struct Pair { int x; int y; };
  int calls = 0;
  SharedPtr<Pair> owner(new Pair{1, 2}, StatefulDeleter{9, &calls});
  SharedPtr<int> alias(owner, &owner->y);
  EXPECT_EQ(get_deleter<StatefulDeleter>(owner), get_deleter<StatefulDeleter>(alias));
}

TEST(SharedPtrGetDeleter, AllowsStateMutation) {
  int calls = 0;
  SharedPtr<int> pointer(new int, StatefulDeleter{1, &calls});
  auto* deleter = get_deleter<StatefulDeleter>(pointer);
  ASSERT_NE(deleter, nullptr);
  deleter->id = 99;
  EXPECT_EQ(get_deleter<StatefulDeleter>(pointer)->id, 99);
}

TEST(SharedPtrNullCustomDeleter, OwnsControlBlockDespiteNullGet) {
  int calls = 0;
  SharedPtr<int> pointer(nullptr, CountingDeleter{&calls});
  EXPECT_EQ(pointer.get(), nullptr);
  EXPECT_EQ(pointer.use_count(), 1);
}

TEST(SharedPtrNullCustomDeleter, CopySharesNullOwnerGroup) {
  int calls = 0;
  SharedPtr<int> first(nullptr, CountingDeleter{&calls});
  SharedPtr<int> second(first);
  EXPECT_EQ(first.use_count(), 2);
  EXPECT_TRUE(SharesOwnership(first, second));
}

TEST(SharedPtrNullCustomDeleter, EmptyPointerHasDifferentOwnership) {
  int calls = 0;
  SharedPtr<int> owned_null(nullptr, CountingDeleter{&calls});
  SharedPtr<int> empty;
  EXPECT_TRUE(DifferentOwnership(owned_null, empty));
}

}  // namespace shared_ptr_tests
