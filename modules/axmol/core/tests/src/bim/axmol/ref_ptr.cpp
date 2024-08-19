// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/ref_ptr.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

#include <axmol/2d/Layer.h>
#include <axmol/2d/Node.h>

#include <utility>

#include <gtest/gtest.h>

template <typename Pair>
class bim_axmol_ref_ptr_test : public testing::Test
{};

using ref_ptr_types = testing::Types<std::pair<ax::Node, ax::Node>,
                                     std::pair<ax::Node, ax::Layer>>;

TYPED_TEST_SUITE(bim_axmol_ref_ptr_test, ref_ptr_types);

TYPED_TEST(bim_axmol_ref_ptr_test, default_constructor)
{
  using target_type = typename TypeParam::first_type;

  const bim::axmol::ref_ptr<target_type> p;
  EXPECT_EQ(nullptr, p.get());
}

TYPED_TEST(bim_axmol_ref_ptr_test, construct_null)
{
  using target_type = typename TypeParam::first_type;

  const bim::axmol::ref_ptr<target_type> p(nullptr);
  EXPECT_EQ(nullptr, p.get());
}

TYPED_TEST(bim_axmol_ref_ptr_test, copy_construct_null)
{
  using target_type = typename TypeParam::first_type;
  using source_type = typename TypeParam::second_type;

  const bim::axmol::ref_ptr<source_type> source;
  const bim::axmol::ref_ptr<target_type> target(source);

  EXPECT_EQ(nullptr, target.get());
}

TYPED_TEST(bim_axmol_ref_ptr_test, copy_construct_non_null)
{
  using target_type = typename TypeParam::first_type;
  using source_type = typename TypeParam::second_type;

  source_type* const ptr = source_type::create();
  const bim::axmol::ref_ptr<source_type> source(ptr);
  ASSERT_EQ(ptr, source.get());
  EXPECT_EQ(2, source->getReferenceCount());

  const bim::axmol::ref_ptr<target_type> target(source);

  EXPECT_EQ(source.get(), target.get());
  EXPECT_EQ(3, target->getReferenceCount());
}

TYPED_TEST(bim_axmol_ref_ptr_test, move_construct_null)
{
  using target_type = typename TypeParam::first_type;
  using source_type = typename TypeParam::second_type;

  bim::axmol::ref_ptr<source_type> source;
  const bim::axmol::ref_ptr<target_type> target(std::move(source));

  EXPECT_EQ(nullptr, source.get());
  EXPECT_EQ(nullptr, target.get());
}

TYPED_TEST(bim_axmol_ref_ptr_test, move_construct_non_null)
{
  using target_type = typename TypeParam::first_type;
  using source_type = typename TypeParam::second_type;

  source_type* const ptr = source_type::create();
  bim::axmol::ref_ptr<source_type> source(ptr);
  EXPECT_EQ(2, source->getReferenceCount());

  const bim::axmol::ref_ptr<target_type> target(std::move(source));

  EXPECT_EQ(ptr, target.get());
  EXPECT_EQ(nullptr, source.get());
  EXPECT_EQ(2, target->getReferenceCount());
}

TYPED_TEST(bim_axmol_ref_ptr_test, release_in_destructor)
{
  using target_type = typename TypeParam::first_type;
  using source_type = typename TypeParam::second_type;

  source_type* const ptr = source_type::create();
  EXPECT_EQ(1, ptr->getReferenceCount());

  {
    const bim::axmol::ref_ptr<source_type> source(ptr);
    EXPECT_EQ(2, ptr->getReferenceCount());

    {
      const bim::axmol::ref_ptr<target_type> target(ptr);
      EXPECT_EQ(3, ptr->getReferenceCount());
    }
    EXPECT_EQ(2, ptr->getReferenceCount());
  }

  EXPECT_EQ(1, ptr->getReferenceCount());
}

TYPED_TEST(bim_axmol_ref_ptr_test, assign_null)
{
  using target_type = typename TypeParam::first_type;

  target_type* const ptr = target_type::create();
  EXPECT_EQ(1, ptr->getReferenceCount());

  bim::axmol::ref_ptr<target_type> target(ptr);
  EXPECT_EQ(2, ptr->getReferenceCount());

  target = nullptr;
  EXPECT_EQ(1, ptr->getReferenceCount());
}

TYPED_TEST(bim_axmol_ref_ptr_test, assign_non_null)
{
  using target_type = typename TypeParam::first_type;
  using source_type = typename TypeParam::second_type;

  target_type* const target_ptr = target_type::create();
  EXPECT_EQ(1, target_ptr->getReferenceCount());

  source_type* const source_ptr = source_type::create();
  EXPECT_EQ(1, source_ptr->getReferenceCount());

  bim::axmol::ref_ptr<target_type> target(target_ptr);
  EXPECT_EQ(2, target_ptr->getReferenceCount());
  EXPECT_EQ(1, source_ptr->getReferenceCount());

  target = source_ptr;
  EXPECT_EQ(source_ptr, target.get());
  EXPECT_EQ(1, target_ptr->getReferenceCount());
  EXPECT_EQ(2, source_ptr->getReferenceCount());
}

TYPED_TEST(bim_axmol_ref_ptr_test, assign_copy_ref_ptr_null)
{
  using target_type = typename TypeParam::first_type;
  using source_type = typename TypeParam::second_type;

  target_type* const ptr = target_type::create();
  EXPECT_EQ(1, ptr->getReferenceCount());

  bim::axmol::ref_ptr<target_type> target(ptr);
  bim::axmol::ref_ptr<source_type> null;
  EXPECT_EQ(2, ptr->getReferenceCount());

  target = null;
  EXPECT_EQ(nullptr, target.get());
  EXPECT_EQ(1, ptr->getReferenceCount());
}

TYPED_TEST(bim_axmol_ref_ptr_test, assign_copy_ref_ptr_non_null)
{
  using target_type = typename TypeParam::first_type;
  using source_type = typename TypeParam::second_type;

  target_type* const target_ptr = target_type::create();
  EXPECT_EQ(1, target_ptr->getReferenceCount());

  source_type* const source_ptr = source_type::create();
  EXPECT_EQ(1, source_ptr->getReferenceCount());

  bim::axmol::ref_ptr<target_type> target(target_ptr);
  bim::axmol::ref_ptr<source_type> source(source_ptr);

  EXPECT_EQ(2, target_ptr->getReferenceCount());
  EXPECT_EQ(2, source_ptr->getReferenceCount());

  target = source;
  EXPECT_EQ(source_ptr, target.get());
  EXPECT_EQ(source.get(), target.get());
  EXPECT_EQ(1, target_ptr->getReferenceCount());
  EXPECT_EQ(3, source_ptr->getReferenceCount());
}

TYPED_TEST(bim_axmol_ref_ptr_test, assign_move_ref_ptr_null)
{
  using target_type = typename TypeParam::first_type;
  using source_type = typename TypeParam::second_type;

  target_type* const ptr = target_type::create();
  EXPECT_EQ(1, ptr->getReferenceCount());

  bim::axmol::ref_ptr<target_type> target(ptr);
  bim::axmol::ref_ptr<source_type> null;
  EXPECT_EQ(2, ptr->getReferenceCount());

  target = std::move(null);
  EXPECT_EQ(nullptr, target.get());
  EXPECT_EQ(1, ptr->getReferenceCount());
}

TYPED_TEST(bim_axmol_ref_ptr_test, assign_move_ref_ptr_non_null)
{
  using target_type = typename TypeParam::first_type;
  using source_type = typename TypeParam::second_type;

  target_type* const target_ptr = target_type::create();
  EXPECT_EQ(1, target_ptr->getReferenceCount());

  source_type* const source_ptr = source_type::create();
  EXPECT_EQ(1, source_ptr->getReferenceCount());

  bim::axmol::ref_ptr<target_type> target(target_ptr);
  bim::axmol::ref_ptr<source_type> source(source_ptr);

  EXPECT_EQ(2, target_ptr->getReferenceCount());
  EXPECT_EQ(2, source_ptr->getReferenceCount());

  target = std::move(source);
  EXPECT_EQ(source_ptr, target.get());
  EXPECT_EQ(nullptr, source.get());
  EXPECT_EQ(1, target_ptr->getReferenceCount());
  EXPECT_EQ(2, source_ptr->getReferenceCount());
  EXPECT_EQ(nullptr, source.get());
}

TYPED_TEST(bim_axmol_ref_ptr_test, equality_and_difference)
{
  using target_type = typename TypeParam::first_type;
  using source_type = typename TypeParam::second_type;

  bim::axmol::ref_ptr<target_type> target(source_type::create());
  bim::axmol::ref_ptr<source_type> source(source_type::create());

  EXPECT_TRUE(target == target);
  EXPECT_TRUE(source == source);

  EXPECT_FALSE(target == source);
  EXPECT_FALSE(source == target);

  EXPECT_FALSE(target != target);
  EXPECT_FALSE(source != source);

  EXPECT_TRUE(target != source);
  EXPECT_TRUE(source != target);

  // Test again but with different types for the same instance.
  target = source;

  EXPECT_TRUE(target == source);
  EXPECT_TRUE(source == target);

  EXPECT_FALSE(target != source);
  EXPECT_FALSE(source != target);
}

TYPED_TEST(bim_axmol_ref_ptr_test, operator_star)
{
  using target_type = typename TypeParam::first_type;
  using source_type = typename TypeParam::second_type;

  const bim::axmol::ref_ptr<target_type> target(source_type::create());

  EXPECT_EQ(target.get(), target.operator->());
  EXPECT_EQ(target.get(), &(*target));
}
