// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/session_service.hpp>

#include <bim/server/service/statistics_service.hpp>

#include <bim/server/tests/fake_scheduler.hpp>
#include <bim/server/tests/new_test_config.hpp>

#include <chrono>

#include <gtest/gtest.h>

TEST(session_service, sessions_and_tokens)
{
  bim::server::tests::fake_scheduler scheduler;

  bim::server::config config = bim::server::tests::new_test_config();
  config.session_clean_up_interval = std::chrono::seconds(1);
  config.session_removal_delay = std::chrono::seconds(5);

  bim::server::statistics_service statistics(config);
  bim::server::session_service service(config, statistics);

  const boost::asio::ip::address_v4 address_0(0x01010101);
  const boost::asio::ip::address_v4 address_1(0x02020202);

  const std::optional<iscool::net::session_id> session_0 =
      service.create_or_refresh_session(address_0, 111);
  ASSERT_TRUE(!!session_0);

  const std::optional<iscool::net::session_id> session_1 =
      service.create_or_refresh_session(address_1, 222);
  ASSERT_TRUE(!!session_1);

  const std::optional<iscool::net::session_id> session_0_bis =
      service.create_or_refresh_session(address_0, 111);
  ASSERT_TRUE(!!session_0_bis);
  EXPECT_EQ(*session_0, *session_0_bis);

  const std::optional<iscool::net::session_id> session_0_ter =
      service.create_or_refresh_session(address_1, 111);
  ASSERT_TRUE(!!session_0_ter);
  EXPECT_EQ(*session_0, *session_0_ter);

  // Trigger the clean up but keep session 1 active, such that only session 0
  // is removed.
  for (int i = 0; i != 6; ++i)
    {
      EXPECT_TRUE(service.refresh_session(*session_1));
      scheduler.tick(std::chrono::seconds(1));
    }

  // session_0 has been removed, same address and token pair creates a new
  // session.
  const std::optional<iscool::net::session_id> new_session_0 =
      service.create_or_refresh_session(address_0, 111);
  ASSERT_TRUE(!!new_session_0);
  EXPECT_NE(*session_0, *new_session_0);

  // session_1 is still active so the same token should return the same
  // session.
  const std::optional<iscool::net::session_id> session_1_bis =
      service.create_or_refresh_session(address_1, 222);
  ASSERT_TRUE(!!session_1_bis);
  EXPECT_EQ(*session_1, *session_1_bis);
}

TEST(session_service, karma)
{
  bim::server::tests::fake_scheduler scheduler;

  bim::server::config config = bim::server::tests::new_test_config();
  config.enable_karma = true;
  config.session_clean_up_interval = std::chrono::hours(50);
  config.session_removal_delay = std::chrono::hours(50);
  config.karma_blacklisting_duration = std::chrono::minutes(10);
  config.karma_review_interval = std::chrono::minutes(5);
  config.initial_karma_value = 10;
  config.disconnection_karma_adjustment = -6;
  config.short_game_karma_adjustment = -3;
  config.good_behavior_karma_adjustment = 3;

  bim::server::statistics_service statistics(config);
  bim::server::session_service service(config, statistics);

  const boost::asio::ip::address_v4 a_1(0x01010101);
  const boost::asio::ip::address_v4 a_2(0x02020202);
  const boost::asio::ip::address_v4 a_3(0x03030303);

  std::optional<iscool::net::session_id> session_1 =
      service.create_or_refresh_session(a_1, 111);
  std::optional<iscool::net::session_id> session_2 =
      service.create_or_refresh_session(a_2, 222);
  const std::optional<iscool::net::session_id> session_3 =
      service.create_or_refresh_session(a_3, 333);

  ASSERT_TRUE(!!session_1);
  ASSERT_TRUE(!!session_2);
  ASSERT_TRUE(!!session_3);

  service.update_karma_disconnection(*session_1);
  // karma = 7
  service.update_karma_short_game(*session_2);

  EXPECT_FALSE(service.refresh_session(*session_1));
  EXPECT_TRUE(service.refresh_session(*session_2));
  EXPECT_TRUE(service.refresh_session(*session_3));

  session_1 = service.create_or_refresh_session(a_1, 444);
  ASSERT_TRUE(!!session_1);

  service.update_karma_disconnection(*session_1);
  // karma = 4
  service.update_karma_short_game(*session_2);

  EXPECT_FALSE(service.refresh_session(*session_1));
  EXPECT_TRUE(service.refresh_session(*session_2));
  EXPECT_TRUE(service.refresh_session(*session_3));

  session_1 = service.create_or_refresh_session(a_1, 444);
  EXPECT_FALSE(!!session_1);

  // Update the karma service, nothing changes.
  scheduler.tick(std::chrono::minutes(5));

  EXPECT_TRUE(service.refresh_session(*session_2));
  EXPECT_TRUE(service.refresh_session(*session_3));

  session_1 = service.create_or_refresh_session(a_1, 444);
  EXPECT_FALSE(!!session_1);

  // karma = 1
  service.update_karma_short_game(*session_2);

  EXPECT_TRUE(service.refresh_session(*session_2));
  EXPECT_TRUE(service.refresh_session(*session_3));

  session_1 = service.create_or_refresh_session(a_1, 444);
  EXPECT_FALSE(!!session_1);

  // karma back to 4
  service.update_karma_good_behavior(*session_2);

  EXPECT_TRUE(service.refresh_session(*session_2));
  EXPECT_TRUE(service.refresh_session(*session_3));

  session_1 = service.create_or_refresh_session(a_1, 444);
  EXPECT_FALSE(!!session_1);

  // karma = 1
  service.update_karma_short_game(*session_2);

  EXPECT_TRUE(service.refresh_session(*session_2));
  EXPECT_TRUE(service.refresh_session(*session_3));

  session_1 = service.create_or_refresh_session(a_1, 444);
  EXPECT_FALSE(!!session_1);

  // karma = -2
  service.update_karma_short_game(*session_2);

  EXPECT_FALSE(service.refresh_session(*session_2));
  EXPECT_TRUE(service.refresh_session(*session_3));

  session_1 = service.create_or_refresh_session(a_1, 444);
  session_2 = service.create_or_refresh_session(a_2, 555);

  EXPECT_FALSE(!!session_1);
  EXPECT_FALSE(!!session_2);

  // Update the service, a_1 is back.
  scheduler.tick(std::chrono::minutes(5));

  session_1 = service.create_or_refresh_session(a_1, 444);
  session_2 = service.create_or_refresh_session(a_2, 555);

  EXPECT_TRUE(!!session_1);
  EXPECT_FALSE(!!session_2);
  EXPECT_TRUE(service.refresh_session(*session_3));

  // Update the service, a_2 is back.
  scheduler.tick(std::chrono::minutes(5));

  session_2 = service.create_or_refresh_session(a_2, 555);

  EXPECT_TRUE(service.refresh_session(*session_1));
  ASSERT_TRUE(!!session_2);
  EXPECT_TRUE(service.refresh_session(*session_3));
}
