#include "test_support.hpp"

namespace shared_ptr_tests {

TEST(SharedPtrLifetime, DestroysSoleOwnedObjectExactlyOnce) {
  LifetimeStats stats;
  { SharedPtr<Tracked> pointer(new Tracked(&stats)); }
  EXPECT_EQ(stats.constructed, 1);
  EXPECT_EQ(stats.destroyed, 1);
}

TEST(SharedPtrLifetime, DoesNotDestroyWhileCopyExists) {
  LifetimeStats stats;
  SharedPtr<Tracked> surviving;
  {
    SharedPtr<Tracked> first(new Tracked(&stats));
    surviving = first;
  }
  EXPECT_EQ(stats.destroyed, 0);
  surviving.reset();
  EXPECT_EQ(stats.destroyed, 1);
}

TEST(SharedPtrLifetime, DestroysOnLastCopyReset) {
  LifetimeStats stats;
  SharedPtr<Tracked> first(new Tracked(&stats));
  SharedPtr<Tracked> second(first);
  SharedPtr<Tracked> third(second);
  first.reset();
  second.reset();
  EXPECT_EQ(stats.destroyed, 0);
  third.reset();
  EXPECT_EQ(stats.destroyed, 1);
}

TEST(SharedPtrLifetime, MoveDoesNotDestroyObject) {
  LifetimeStats stats;
  SharedPtr<Tracked> first(new Tracked(&stats));
  SharedPtr<Tracked> second(std::move(first));
  EXPECT_EQ(stats.destroyed, 0);
  second.reset();
  EXPECT_EQ(stats.destroyed, 1);
}

TEST(SharedPtrLifetime, CopyAssignmentKeepsSourceAlive) {
  LifetimeStats stats;
  SharedPtr<Tracked> source(new Tracked(&stats));
  SharedPtr<Tracked> target;
  target = source;
  source.reset();
  EXPECT_EQ(stats.destroyed, 0);
  target.reset();
  EXPECT_EQ(stats.destroyed, 1);
}

TEST(SharedPtrLifetime, MoveAssignmentTransfersLifetime) {
  LifetimeStats stats;
  SharedPtr<Tracked> source(new Tracked(&stats));
  SharedPtr<Tracked> target;
  target = std::move(source);
  EXPECT_EQ(stats.destroyed, 0);
  target.reset();
  EXPECT_EQ(stats.destroyed, 1);
}

TEST(SharedPtrLifetime, DestructorDoesNotCopyElement) {
  LifetimeStats stats;
  { SharedPtr<Tracked> pointer(new Tracked(&stats)); }
  EXPECT_EQ(stats.copied, 0);
}

TEST(SharedPtrLifetime, CopyingPointerDoesNotCopyElement) {
  LifetimeStats stats;
  SharedPtr<Tracked> first(new Tracked(&stats));
  SharedPtr<Tracked> second(first);
  SharedPtr<Tracked> third(second);
  EXPECT_EQ(stats.copied, 0);
  EXPECT_EQ(stats.constructed, 1);
}

TEST(SharedPtrLifetime, MovingPointerDoesNotMoveElement) {
  LifetimeStats stats;
  SharedPtr<Tracked> first(new Tracked(&stats));
  SharedPtr<Tracked> second(std::move(first));
  EXPECT_EQ(stats.moved, 0);
  EXPECT_EQ(stats.constructed, 1);
}

TEST(SharedPtrLifetime, CustomDeleterRunsAfterFinalOwner) {
  int calls = 0;
  SharedPtr<int> first(new int, CountingDeleter{&calls});
  {
    SharedPtr<int> second(first);
    EXPECT_EQ(calls, 0);
  }
  EXPECT_EQ(calls, 0);
  first.reset();
  EXPECT_EQ(calls, 1);
}

TEST(SharedPtrLifetime, ResetToNewObjectDestroysOldObjectFirst) {
  std::vector<int> events;
  struct OrderedDeleter {
    std::vector<int>* events;
    int id;
    void operator()(int* p) const { events->push_back(id); delete p; }
  };
  SharedPtr<int> pointer(new int, OrderedDeleter{&events, 1});
  pointer.reset(new int, OrderedDeleter{&events, 2});
  ASSERT_EQ(events.size(), 1u);
  EXPECT_EQ(events[0], 1);
  pointer.reset();
  ASSERT_EQ(events.size(), 2u);
  EXPECT_EQ(events[1], 2);
}

TEST(SharedPtrLifetime, MultipleIndependentObjectsDestroyedIndependently) {
  LifetimeStats first;
  LifetimeStats second;
  {
    SharedPtr<Tracked> a(new Tracked(&first));
    SharedPtr<Tracked> b(new Tracked(&second));
    a.reset();
    EXPECT_EQ(first.destroyed, 1);
    EXPECT_EQ(second.destroyed, 0);
  }
  EXPECT_EQ(second.destroyed, 1);
}

TEST(SharedPtrLifetime, DeepCopyChainDestroysOnce) {
  LifetimeStats stats;
  SharedPtr<Tracked> root(new Tracked(&stats));
  std::vector<SharedPtr<Tracked>> copies(100, root);
  EXPECT_EQ(root.use_count(), 101);
  copies.clear();
  EXPECT_EQ(stats.destroyed, 0);
  root.reset();
  EXPECT_EQ(stats.destroyed, 1);
}

TEST(SharedPtrLifetime, ReverseDestructionOrderIsSafe) {
  LifetimeStats stats;
  std::vector<SharedPtr<Tracked>> owners;
  owners.emplace_back(new Tracked(&stats));
  for (int i = 0; i < 20; ++i) owners.push_back(owners.back());
  while (!owners.empty()) owners.pop_back();
  EXPECT_EQ(stats.destroyed, 1);
}

TEST(SharedPtrLifetime, NullEmptyPointerHasNoDeletionEffect) {
  LifetimeStats stats;
  { SharedPtr<Tracked> pointer; }
  EXPECT_EQ(stats.destroyed, 0);
}

TEST(SharedPtrLifetime, RawNullPointerWithDeleterInvokesDeleterOnce) {
  int calls = 0;
  bool was_null = false;
  { SharedPtr<int> pointer(nullptr, NullAwareDeleter{&calls, &was_null}); }
  EXPECT_EQ(calls, 1);
  EXPECT_TRUE(was_null);
}

TEST(SharedPtrLifetime, AliasingOwnerKeepsCompleteObjectAlive) {
  LifetimeStats stats;
  SharedPtr<Tracked> owner(new Tracked(&stats, 5));
  SharedPtr<int> alias(owner, &owner->value);
  owner.reset();
  EXPECT_EQ(stats.destroyed, 0);
  EXPECT_EQ(*alias, 5);
  alias.reset();
  EXPECT_EQ(stats.destroyed, 1);
}

TEST(SharedPtrLifetime, DerivedDestructorRunsThroughBasePointer) {
  struct D : Base {
    explicit D(bool* flag) : flag(flag) {}
    ~D() override { *flag = true; }
    bool* flag;
  };
  bool destroyed = false;
  { SharedPtr<Base> pointer(new D(&destroyed)); }
  EXPECT_TRUE(destroyed);
}

TEST(SharedPtrLifetime, NonVirtualBaseStillUsesOriginalDerivedDeleter) {
  struct B {};
  struct D : B {
    explicit D(bool* flag) : flag(flag) {}
    ~D() { *flag = true; }
    bool* flag;
  };
  bool destroyed = false;
  { SharedPtr<B> pointer(new D(&destroyed)); }
  EXPECT_TRUE(destroyed);
}

TEST(SharedPtrLifetime, ScopeExitUpdatesAllRemainingCounts) {
  SharedPtr<int> root(new int);
  SharedPtr<int> outside;
  {
    SharedPtr<int> a(root);
    SharedPtr<int> b(a);
    outside = b;
    EXPECT_EQ(root.use_count(), 4);
  }
  EXPECT_EQ(root.use_count(), 2);
  EXPECT_EQ(outside.use_count(), 2);
}

}  // namespace shared_ptr_tests
