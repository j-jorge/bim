// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/karma_service.hpp>

#include <bim/server/tests/fake_scheduler.hpp>
#include <bim/server/tests/new_test_config.hpp>

#include <chrono>

#include <gtest/gtest.h>

TEST(karma_service, enabled)
{
  bim::server::tests::fake_scheduler scheduler;

  bim::server::config config = bim::server::tests::new_test_config();
  config.enable_karma = true;
  config.karma_blacklisting_duration = std::chrono::minutes(10);
  config.karma_review_interval = std::chrono::minutes(5);
  config.initial_karma_value = 10;
  config.disconnection_karma_adjustment = -6;
  config.short_game_karma_adjustment = -3;
  config.good_behavior_karma_adjustment = 3;

  bim::server::karma_service karma(config);

  const boost::asio::ip::address_v4 a_1(0x01010101);
  const boost::asio::ip::address_v4 a_2(0x02020202);
  const boost::asio::ip::address_v4 a_3(0x03030303);

  EXPECT_TRUE(karma.allowed(a_1));
  EXPECT_TRUE(karma.allowed(a_2));
  EXPECT_TRUE(karma.allowed(a_3));

  karma.disconnection(a_1);
  // karma = 7
  karma.short_game(a_2);

  EXPECT_TRUE(karma.allowed(a_1));
  EXPECT_TRUE(karma.allowed(a_2));
  EXPECT_TRUE(karma.allowed(a_3));

  karma.disconnection(a_1);
  // karma = 4
  karma.short_game(a_2);

  EXPECT_FALSE(karma.allowed(a_1));
  EXPECT_TRUE(karma.allowed(a_2));
  EXPECT_TRUE(karma.allowed(a_3));

  // Update the service, nothing changes.
  scheduler.tick(std::chrono::minutes(5));

  EXPECT_FALSE(karma.allowed(a_1));
  EXPECT_TRUE(karma.allowed(a_2));
  EXPECT_TRUE(karma.allowed(a_3));

  // karma = 1
  karma.short_game(a_2);

  EXPECT_FALSE(karma.allowed(a_1));
  EXPECT_TRUE(karma.allowed(a_2));
  EXPECT_TRUE(karma.allowed(a_3));

  // karma back to 4
  karma.good_behavior(a_2);

  EXPECT_FALSE(karma.allowed(a_1));
  EXPECT_TRUE(karma.allowed(a_2));
  EXPECT_TRUE(karma.allowed(a_3));

  // karma = 1
  karma.short_game(a_2);

  EXPECT_FALSE(karma.allowed(a_1));
  EXPECT_TRUE(karma.allowed(a_2));
  EXPECT_TRUE(karma.allowed(a_3));

  // karma = -2
  karma.short_game(a_2);

  EXPECT_FALSE(karma.allowed(a_1));
  EXPECT_FALSE(karma.allowed(a_2));
  EXPECT_TRUE(karma.allowed(a_3));

  // Update the service, a_1 is back.
  scheduler.tick(std::chrono::minutes(5));

  EXPECT_TRUE(karma.allowed(a_1));
  EXPECT_FALSE(karma.allowed(a_2));
  EXPECT_TRUE(karma.allowed(a_3));

  // Update the service, a_2 is back.
  scheduler.tick(std::chrono::minutes(5));

  EXPECT_TRUE(karma.allowed(a_1));
  EXPECT_TRUE(karma.allowed(a_2));
  EXPECT_TRUE(karma.allowed(a_3));
}

TEST(karma_service, disabled)
{
  bim::server::tests::fake_scheduler scheduler;

  bim::server::config config = bim::server::tests::new_test_config();
  config.enable_karma = false;
  config.karma_blacklisting_duration = std::chrono::minutes(10);
  config.karma_review_interval = std::chrono::minutes(5);
  config.initial_karma_value = 10;
  config.disconnection_karma_adjustment = -20;
  config.short_game_karma_adjustment = -30;
  config.good_behavior_karma_adjustment = 3;

  bim::server::karma_service karma(config);

  const boost::asio::ip::address_v4 a_1(0x01010101);
  const boost::asio::ip::address_v4 a_2(0x02020202);
  const boost::asio::ip::address_v4 a_3(0x03030303);

  EXPECT_TRUE(karma.allowed(a_1));
  EXPECT_TRUE(karma.allowed(a_2));
  EXPECT_TRUE(karma.allowed(a_3));

  karma.disconnection(a_1);
  karma.short_game(a_2);

  EXPECT_TRUE(karma.allowed(a_1));
  EXPECT_TRUE(karma.allowed(a_2));
  EXPECT_TRUE(karma.allowed(a_3));
}
