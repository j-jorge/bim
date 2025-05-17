// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/table_2d.impl.hpp>

#include <gtest/gtest.h>

TEST(bim_table_2d, write_read)
{
  bim::table_2d<int> t(3, 2);
  t(0, 1) = 24;

  EXPECT_EQ(3, t.width());
  EXPECT_EQ(2, t.height());
  EXPECT_EQ(24, t(0, 1));
}

TEST(bim_table_2d, init)
{
  bim::table_2d<int> t(3, 2, 24);

  EXPECT_EQ(24, t(0, 0));
  EXPECT_EQ(24, t(1, 0));
  EXPECT_EQ(24, t(2, 0));

  EXPECT_EQ(24, t(0, 1));
  EXPECT_EQ(24, t(1, 1));
  EXPECT_EQ(24, t(2, 1));
}

TEST(bim_table_2d, copy)
{
  const bim::table_2d<int> t(3, 2, 42);
  const bim::table_2d<int> copied(t);

  EXPECT_EQ(42, copied(0, 0));
  EXPECT_EQ(42, copied(1, 0));
  EXPECT_EQ(42, copied(2, 0));

  EXPECT_EQ(42, copied(0, 1));
  EXPECT_EQ(42, copied(1, 1));
  EXPECT_EQ(42, copied(2, 1));
}

TEST(bim_table_2d, move)
{
  bim::table_2d<int> t(3, 2, 11);
  const int* const p = &t(0, 0);
  const bim::table_2d<int> moved(std::move(t));

  EXPECT_EQ(p, &moved(0, 0));
  EXPECT_EQ(11, moved(0, 0));
  EXPECT_EQ(11, moved(1, 0));
  EXPECT_EQ(11, moved(2, 0));

  EXPECT_EQ(11, moved(0, 1));
  EXPECT_EQ(11, moved(1, 1));
  EXPECT_EQ(11, moved(2, 1));
}

TEST(bim_table_2d, copy_assign_realloc)
{
  const bim::table_2d<int> t(3, 2, 23);
  bim::table_2d<int> copied(1, 1);
  const int* const p = &copied(0, 0);
  copied = t;

  EXPECT_NE(p, &copied(0, 0));

  EXPECT_EQ(23, copied(0, 0));
  EXPECT_EQ(23, copied(1, 0));
  EXPECT_EQ(23, copied(2, 0));

  EXPECT_EQ(23, copied(0, 1));
  EXPECT_EQ(23, copied(1, 1));
  EXPECT_EQ(23, copied(2, 1));
}

TEST(bim_table_2d, copy_assign_reuse_storage)
{
  const bim::table_2d<int> t(3, 2, -12);
  bim::table_2d<int> copied(10, 10);
  const int* const p = &copied(0, 0);
  copied = t;

  EXPECT_EQ(p, &copied(0, 0));

  EXPECT_EQ(-12, copied(0, 0));
  EXPECT_EQ(-12, copied(1, 0));
  EXPECT_EQ(-12, copied(2, 0));

  EXPECT_EQ(-12, copied(0, 1));
  EXPECT_EQ(-12, copied(1, 1));
  EXPECT_EQ(-12, copied(2, 1));
}

TEST(bim_table_2d, move_assign)
{
  bim::table_2d<int> t(3, 2, 99);
  const int* const p = &t(0, 0);
  bim::table_2d<int> moved;

  moved = std::move(t);

  EXPECT_EQ(p, &moved(0, 0));

  EXPECT_EQ(99, moved(0, 0));
  EXPECT_EQ(99, moved(1, 0));
  EXPECT_EQ(99, moved(2, 0));

  EXPECT_EQ(99, moved(0, 1));
  EXPECT_EQ(99, moved(1, 1));
  EXPECT_EQ(99, moved(2, 1));
}
