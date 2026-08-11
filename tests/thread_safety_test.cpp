#include "test_support.hpp"

#include "shared_ptr/weak_ptr.hpp"

namespace shared_ptr_tests {

TEST(SharedPtrThreadSafety, ConcurrentCopiesOfSameOwnerAreSafe) {
  SharedPtr<int> owner(new int(42));
  std::atomic<bool> ok{true};
  std::vector<std::thread> threads;
  for (int t = 0; t < 8; ++t) {
    threads.emplace_back([&] {
      for (int i = 0; i < 5000; ++i) {
        SharedPtr<int> copy(owner);
        if (!copy || *copy != 42) ok.store(false, std::memory_order_relaxed);
      }
    });
  }
  for (auto& thread : threads) thread.join();
  EXPECT_TRUE(ok.load());
  EXPECT_EQ(owner.use_count(), 1);
}

TEST(SharedPtrThreadSafety, ConcurrentIndependentResetsAreSafe) {
  SharedPtr<int> owner(new int(17));
  std::vector<SharedPtr<int>> copies(16, owner);
  std::vector<std::thread> threads;
  for (auto& copy : copies) {
    threads.emplace_back([&copy] { copy.reset(); });
  }
  for (auto& thread : threads) thread.join();
  EXPECT_EQ(owner.use_count(), 1);
  EXPECT_EQ(*owner, 17);
}

TEST(SharedPtrThreadSafety, ConcurrentFinalReleasesDestroyExactlyOnce) {
  std::atomic<int> deletes{0};
  struct AtomicDeleter {
    std::atomic<int>* deletes;
    void operator()(int* p) const {
      deletes->fetch_add(1, std::memory_order_relaxed);
      delete p;
    }
  };
  SharedPtr<int> owner(new int, AtomicDeleter{&deletes});
  std::vector<SharedPtr<int>> copies(16, owner);
  owner.reset();
  std::vector<std::thread> threads;
  for (auto& copy : copies) {
    threads.emplace_back([&copy] { copy.reset(); });
  }
  for (auto& thread : threads) thread.join();
  EXPECT_EQ(deletes.load(), 1);
}

TEST(SharedPtrThreadSafety, ConcurrentCopyMoveCyclesKeepCountBalanced) {
  SharedPtr<int> owner(new int(9));
  std::vector<std::thread> threads;
  for (int t = 0; t < 8; ++t) {
    threads.emplace_back([&] {
      for (int i = 0; i < 3000; ++i) {
        SharedPtr<int> first(owner);
        SharedPtr<int> second(std::move(first));
        first = second;
        second.reset();
      }
    });
  }
  for (auto& thread : threads) thread.join();
  EXPECT_EQ(owner.use_count(), 1);
}

TEST(SharedPtrThreadSafety, ConcurrentWeakLocksKeepObjectValid) {
  SharedPtr<int> owner(new int(123));
  WeakPtr<int> weak(owner);
  std::atomic<bool> ok{true};
  std::vector<std::thread> threads;
  for (int t = 0; t < 8; ++t) {
    threads.emplace_back([&] {
      for (int i = 0; i < 3000; ++i) {
        auto locked = weak.lock();
        if (!locked || *locked != 123) ok.store(false, std::memory_order_relaxed);
      }
    });
  }
  for (auto& thread : threads) thread.join();
  EXPECT_TRUE(ok.load());
  EXPECT_EQ(owner.use_count(), 1);
}

TEST(SharedPtrThreadSafety, WeakLockRacesWithLastReleaseSafely) {
  SharedPtr<int> owner(new int(51));
  WeakPtr<int> weak(owner);
  std::atomic<bool> start{false};
  std::atomic<bool> valid_values{true};
  std::thread locker([&] {
    while (!start.load(std::memory_order_acquire)) {}
    for (int i = 0; i < 10000; ++i) {
      auto locked = weak.lock();
      if (locked && *locked != 51) valid_values.store(false, std::memory_order_relaxed);
    }
  });
  std::thread releaser([&] {
    start.store(true, std::memory_order_release);
    owner.reset();
  });
  locker.join();
  releaser.join();
  EXPECT_TRUE(valid_values.load());
  EXPECT_TRUE(weak.expired());
}

TEST(SharedPtrThreadSafety, IndependentOwnerGroupsDoNotInterfere) {
  std::array<SharedPtr<int>, 8> owners;
  for (int i = 0; i < 8; ++i) owners[i].reset(new int(i));
  std::atomic<bool> ok{true};
  std::vector<std::thread> threads;
  for (int i = 0; i < 8; ++i) {
    threads.emplace_back([&, i] {
      for (int n = 0; n < 4000; ++n) {
        SharedPtr<int> copy(owners[i]);
        if (*copy != i) ok.store(false, std::memory_order_relaxed);
      }
    });
  }
  for (auto& thread : threads) thread.join();
  EXPECT_TRUE(ok.load());
  for (const auto& owner : owners) EXPECT_EQ(owner.use_count(), 1);
}

TEST(SharedPtrThreadSafety, ConcurrentAliasingCopiesPreserveStoredAddress) {
  struct Pair { int first = 1; int second = 2; };
  SharedPtr<Pair> owner(new Pair);
  SharedPtr<int> alias(owner, &owner->second);
  int* expected = alias.get();
  std::atomic<bool> ok{true};
  std::vector<std::thread> threads;
  for (int t = 0; t < 8; ++t) {
    threads.emplace_back([&] {
      for (int i = 0; i < 3000; ++i) {
        SharedPtr<int> copy(alias);
        if (copy.get() != expected || *copy != 2) ok.store(false, std::memory_order_relaxed);
      }
    });
  }
  for (auto& thread : threads) thread.join();
  EXPECT_TRUE(ok.load());
  EXPECT_EQ(owner.use_count(), 2);
}

TEST(SharedPtrThreadSafety, ConcurrentCustomDeleterOwnerCopiesDeleteOnce) {
  std::atomic<int> calls{0};
  struct Deleter {
    std::atomic<int>* calls;
    void operator()(int* p) const { calls->fetch_add(1); delete p; }
  };
  SharedPtr<int> owner(new int, Deleter{&calls});
  std::vector<std::thread> threads;
  for (int t = 0; t < 8; ++t) {
    threads.emplace_back([owner]() mutable {
      for (int i = 0; i < 2000; ++i) {
        SharedPtr<int> local(owner);
      }
      owner.reset();
    });
  }
  owner.reset();
  for (auto& thread : threads) thread.join();
  EXPECT_EQ(calls.load(), 1);
}

TEST(SharedPtrThreadSafety, LargeFanOutReturnsToSingleOwner) {
  SharedPtr<int> owner(new int(5));
  std::vector<std::thread> threads;
  for (int t = 0; t < 32; ++t) {
    threads.emplace_back([owner] {
      std::vector<SharedPtr<int>> local(100, owner);
    });
  }
  for (auto& thread : threads) thread.join();
  EXPECT_EQ(owner.use_count(), 1);
}

}  // namespace shared_ptr_tests
