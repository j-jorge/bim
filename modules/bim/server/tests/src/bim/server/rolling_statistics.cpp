// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/rolling_statistics.hpp>

#include <gtest/gtest.h>

TEST(rolling_statistics_test, empty_is_zero)
{
  bim::server::rolling_statistics stats(std::chrono::minutes(1),
                                        std::chrono::minutes(10));
  EXPECT_EQ(0, stats.total());
}

TEST(rolling_statistics_test, push_full)
{
  bim::server::rolling_statistics stats(std::chrono::minutes(1),
                                        std::chrono::minutes(10));

  for (int i = 0; i <= 10; ++i)
    {
      stats.push(std::chrono::minutes(i), 1);
      EXPECT_EQ(i + 1, stats.total());
    }

  for (int i = 11; i != 20; ++i)
    {
      stats.push(std::chrono::minutes(i), 1);
      EXPECT_EQ(11, stats.total());
    }
}

TEST(rolling_statistics_test, push_aggregate)
{
  bim::server::rolling_statistics stats(std::chrono::minutes(1),
                                        std::chrono::minutes(10));

  stats.push(std::chrono::minutes(0), 1);
  stats.push(std::chrono::minutes(0), 1);
  stats.push(std::chrono::minutes(0), 1);
  EXPECT_EQ(3, stats.total());

  for (int i = 1; i <= 10; ++i)
    {
      stats.push(std::chrono::minutes(i), 1);
      EXPECT_EQ(3 + i, stats.total());
    }

  stats.push(std::chrono::minutes(10), 1);
  EXPECT_EQ(3 + 10 + 1, stats.total());

  stats.push(std::chrono::minutes(11), 1);
  EXPECT_EQ(10 + 1 + 1, stats.total());
}

TEST(rolling_statistics_test, ignore_non_increasing_date)
{
  bim::server::rolling_statistics stats(std::chrono::minutes(1),
                                        std::chrono::minutes(10));

  stats.push(std::chrono::minutes(12), 1);
  stats.push(std::chrono::minutes(7), 1);
  EXPECT_EQ(1, stats.total());
}
