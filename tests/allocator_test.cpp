#include "test_support.hpp"

#include "shared_ptr/weak_ptr.hpp"

namespace shared_ptr_tests {

TEST(SharedPtrAllocatorConstructor, UsesAllocatorForControlBlock) {
  AllocationStats stats;
  {
    SharedPtr<int> pointer(new int(7), std::default_delete<int>{},
                           CountingAllocator<std::byte>(&stats));
    EXPECT_GE(stats.allocations.load(), 1);
    EXPECT_EQ(*pointer, 7);
  }
}

TEST(SharedPtrAllocatorConstructor, DeallocatesControlBlock) {
  AllocationStats stats;
  {
    SharedPtr<int> pointer(new int(7), std::default_delete<int>{},
                           CountingAllocator<std::byte>(&stats));
  }
  EXPECT_EQ(stats.allocations.load(), stats.deallocations.load());
}

TEST(SharedPtrAllocatorConstructor, InvokesProvidedDeleter) {
  AllocationStats stats;
  int calls = 0;
  {
    SharedPtr<int> pointer(new int(7), CountingDeleter{&calls},
                           CountingAllocator<std::byte>(&stats));
  }
  EXPECT_EQ(calls, 1);
}

TEST(SharedPtrAllocatorConstructor, CopyDoesNotAllocateAnotherControlBlock) {
  AllocationStats stats;
  SharedPtr<int> first(new int(7), std::default_delete<int>{},
                       CountingAllocator<std::byte>(&stats));
  const int allocations = stats.allocations.load();
  SharedPtr<int> second(first);
  EXPECT_EQ(stats.allocations.load(), allocations);
  EXPECT_EQ(second.use_count(), 2);
}

TEST(SharedPtrAllocatorConstructor, MoveDoesNotAllocateAnotherControlBlock) {
  AllocationStats stats;
  SharedPtr<int> first(new int(7), std::default_delete<int>{},
                       CountingAllocator<std::byte>(&stats));
  const int allocations = stats.allocations.load();
  SharedPtr<int> second(std::move(first));
  EXPECT_EQ(stats.allocations.load(), allocations);
  EXPECT_EQ(second.use_count(), 1);
}

TEST(SharedPtrAllocatorConstructor, NullPointerStillAllocatesControlBlock) {
  AllocationStats stats;
  int calls = 0;
  {
    SharedPtr<int> pointer(nullptr, CountingDeleter{&calls},
                           CountingAllocator<std::byte>(&stats));
    EXPECT_EQ(pointer.use_count(), 1);
    EXPECT_GE(stats.allocations.load(), 1);
  }
  EXPECT_EQ(calls, 1);
}

TEST(SharedPtrAllocatorConstructor, WeakOwnerDelaysControlBlockDeallocation) {
  AllocationStats stats;
  WeakPtr<int> weak;
  {
    SharedPtr<int> pointer(new int(7), std::default_delete<int>{},
                           CountingAllocator<std::byte>(&stats));
    weak = pointer;
  }
  EXPECT_EQ(stats.deallocations.load(), 0);
  weak.reset();
  EXPECT_EQ(stats.allocations.load(), stats.deallocations.load());
}

TEST(SharedPtrAllocatorConstructor, ResetDestroysOldAllocatorControlBlock) {
  AllocationStats stats;
  SharedPtr<int> pointer(new int(7), std::default_delete<int>{},
                         CountingAllocator<std::byte>(&stats));
  pointer.reset();
  EXPECT_EQ(stats.allocations.load(), stats.deallocations.load());
}

TEST(SharedPtrAllocatorReset, UsesAllocatorForReplacementControlBlock) {
  AllocationStats stats;
  SharedPtr<int> pointer;
  pointer.reset(new int(8), std::default_delete<int>{},
                CountingAllocator<std::byte>(&stats));
  EXPECT_GE(stats.allocations.load(), 1);
  EXPECT_EQ(*pointer, 8);
}

TEST(SharedPtrAllocatorReset, BalancesAllocationAndDeallocation) {
  AllocationStats stats;
  {
    SharedPtr<int> pointer;
    pointer.reset(new int(8), std::default_delete<int>{},
                  CountingAllocator<std::byte>(&stats));
  }
  EXPECT_EQ(stats.allocations.load(), stats.deallocations.load());
}

TEST(SharedPtrAllocatorReset, UsesCustomDeleter) {
  AllocationStats stats;
  int calls = 0;
  {
    SharedPtr<int> pointer;
    pointer.reset(new int(8), CountingDeleter{&calls},
                  CountingAllocator<std::byte>(&stats));
  }
  EXPECT_EQ(calls, 1);
}

TEST(SharedPtrAllocatorReset, ReplacesOldOwnerGroup) {
  AllocationStats first_stats;
  AllocationStats second_stats;
  SharedPtr<int> pointer(new int(1), std::default_delete<int>{},
                         CountingAllocator<std::byte>(&first_stats));
  pointer.reset(new int(2), std::default_delete<int>{},
                CountingAllocator<std::byte>(&second_stats));
  EXPECT_EQ(first_stats.allocations.load(), first_stats.deallocations.load());
  EXPECT_EQ(*pointer, 2);
}

TEST(SharedPtrAllocatorExceptionSafety, AllocationFailureInvokesDeleter) {
  int calls = 0;
  EXPECT_THROW(
      (SharedPtr<int>(new int(3), CountingDeleter{&calls},
                      FailingAllocator<std::byte>{})),
      std::bad_alloc);
  EXPECT_EQ(calls, 1);
}

}  // namespace shared_ptr_tests
