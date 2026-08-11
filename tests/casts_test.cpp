#include "test_support.hpp"

namespace shared_ptr_tests {

TEST(SharedPtrStaticPointerCast, DowncastsKnownDerivedObject) {
  SharedPtr<Base> base(new Derived);
  auto derived = static_pointer_cast<Derived>(base);
  ASSERT_TRUE(derived);
  EXPECT_EQ(derived->derived_value, 20);
}

TEST(SharedPtrStaticPointerCast, SharesOwnership) {
  SharedPtr<Base> base(new Derived);
  auto derived = static_pointer_cast<Derived>(base);
  EXPECT_TRUE(SharesOwnership(base, derived));
  EXPECT_EQ(base.use_count(), 2);
}

TEST(SharedPtrStaticPointerCast, PreservesAdjustedPointer) {
  SharedPtr<MultiplyInherited> object(new MultiplyInherited);
  SharedPtr<Right> right(object);
  auto complete = static_pointer_cast<MultiplyInherited>(right);
  EXPECT_EQ(complete.get(), object.get());
}

TEST(SharedPtrStaticPointerCast, EmptyInputProducesEmptyOutput) {
  SharedPtr<Derived> empty;
  auto base = static_pointer_cast<Base>(empty);
  ExpectEmpty(base);
}

TEST(SharedPtrStaticPointerCast, RvalueTransfersOwnership) {
  SharedPtr<Derived> source(new Derived);
  auto result = static_pointer_cast<Base>(std::move(source));
  ExpectEmpty(source);
  EXPECT_EQ(result.use_count(), 1);
}

TEST(SharedPtrDynamicPointerCast, SuccessfulDowncastReturnsObject) {
  SharedPtr<Base> base(new Derived);
  auto derived = dynamic_pointer_cast<Derived>(base);
  ASSERT_TRUE(derived);
  EXPECT_EQ(derived->Kind(), 2);
}

TEST(SharedPtrDynamicPointerCast, SuccessfulDowncastSharesOwnership) {
  SharedPtr<Base> base(new Derived);
  auto derived = dynamic_pointer_cast<Derived>(base);
  EXPECT_TRUE(SharesOwnership(base, derived));
  EXPECT_EQ(base.use_count(), 2);
}

TEST(SharedPtrDynamicPointerCast, FailedDowncastReturnsEmpty) {
  SharedPtr<Base> base(new OtherDerived);
  auto derived = dynamic_pointer_cast<Derived>(base);
  ExpectEmpty(derived);
  EXPECT_EQ(base.use_count(), 1);
}

TEST(SharedPtrDynamicPointerCast, EmptyInputProducesEmptyOutput) {
  SharedPtr<Base> base;
  auto derived = dynamic_pointer_cast<Derived>(base);
  ExpectEmpty(derived);
}

TEST(SharedPtrDynamicPointerCast, CrossCastWorks) {
  SharedPtr<Left> left(new MultiplyInherited);
  auto right = dynamic_pointer_cast<Right>(left);
  ASSERT_TRUE(right);
  EXPECT_EQ(right->right, 22);
  EXPECT_TRUE(SharesOwnership(left, right));
}

TEST(SharedPtrDynamicPointerCast, SuccessfulRvalueCastTransfersOwnership) {
  SharedPtr<Base> source(new Derived);
  auto result = dynamic_pointer_cast<Derived>(std::move(source));
  ExpectEmpty(source);
  EXPECT_EQ(result.use_count(), 1);
}

TEST(SharedPtrDynamicPointerCast, FailedRvalueCastKeepsSourceOwnership) {
  SharedPtr<Base> source(new OtherDerived);
  auto result = dynamic_pointer_cast<Derived>(std::move(source));
  ExpectEmpty(result);
  EXPECT_TRUE(source);
  EXPECT_EQ(source.use_count(), 1);
}

TEST(SharedPtrConstPointerCast, RemovesConstQualification) {
  SharedPtr<const int> source(new int(15));
  auto mutable_pointer = const_pointer_cast<int>(source);
  *mutable_pointer = 18;
  EXPECT_EQ(*source, 18);
}

TEST(SharedPtrConstPointerCast, AddsConstQualification) {
  SharedPtr<int> source(new int(15));
  auto const_pointer = const_pointer_cast<const int>(source);
  EXPECT_EQ(*const_pointer, 15);
}

TEST(SharedPtrConstPointerCast, SharesOwnership) {
  SharedPtr<const int> source(new int(15));
  auto result = const_pointer_cast<int>(source);
  EXPECT_TRUE(SharesOwnership(source, result));
  EXPECT_EQ(source.use_count(), 2);
}

TEST(SharedPtrConstPointerCast, EmptyInputProducesEmptyOutput) {
  SharedPtr<const int> source;
  auto result = const_pointer_cast<int>(source);
  ExpectEmpty(result);
}

TEST(SharedPtrConstPointerCast, RvalueTransfersOwnership) {
  SharedPtr<const int> source(new int(15));
  auto result = const_pointer_cast<int>(std::move(source));
  ExpectEmpty(source);
  EXPECT_EQ(result.use_count(), 1);
}

TEST(SharedPtrReinterpretPointerCast, PreservesAddress) {
  SharedPtr<std::uint32_t> source(new std::uint32_t(0x12345678u));
  auto bytes = reinterpret_pointer_cast<unsigned char>(source);
  EXPECT_EQ(static_cast<const void*>(bytes.get()), static_cast<const void*>(source.get()));
}

TEST(SharedPtrReinterpretPointerCast, SharesOwnership) {
  SharedPtr<std::uint32_t> source(new std::uint32_t(1));
  auto bytes = reinterpret_pointer_cast<unsigned char>(source);
  EXPECT_TRUE(SharesOwnership(source, bytes));
  EXPECT_EQ(source.use_count(), 2);
}

TEST(SharedPtrReinterpretPointerCast, EmptyInputProducesEmptyOutput) {
  SharedPtr<std::uint32_t> source;
  auto bytes = reinterpret_pointer_cast<unsigned char>(source);
  ExpectEmpty(bytes);
}

TEST(SharedPtrReinterpretPointerCast, RvalueTransfersOwnership) {
  SharedPtr<std::uint32_t> source(new std::uint32_t(1));
  auto bytes = reinterpret_pointer_cast<unsigned char>(std::move(source));
  ExpectEmpty(source);
  EXPECT_EQ(bytes.use_count(), 1);
}

}  // namespace shared_ptr_tests
