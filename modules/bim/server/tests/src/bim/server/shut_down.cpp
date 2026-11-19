// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/client_server_simulator.hpp>

#include <bim/server/tests/new_test_config.hpp>

#include <bim/net/message/authentication_error_code.hpp>

#include <bim/game/component/player_action_queue.hpp>
#include <bim/game/component/player_movement.hpp>
#include <bim/game/player_action.hpp>

#include <gtest/gtest.h>

TEST(shut_down_test, can_shut_down_when_no_game)
{
  constexpr std::uint8_t player_count = 2;
  const bim::server::config config = bim::server::tests::new_test_config();
  bim::server::tests::client_server_simulator simulator(player_count, config);

  simulator.authenticate();
  simulator.server.shut_down();

  EXPECT_FALSE(simulator.server.has_any_activity());
}

TEST(shut_down_test, can_shut_down_after_game_over)
{
  constexpr std::uint8_t player_count = 2;
  bim::server::config config = bim::server::tests::new_test_config();
  config.game_service_clean_up_interval = std::chrono::seconds(20);

  bim::server::tests::client_server_simulator simulator(player_count, config);

  simulator.authenticate();
  simulator.join_game();
  simulator.server.shut_down();

  EXPECT_TRUE(simulator.server.has_any_activity());

  simulator.drop_bombs(1);
  simulator.wait_game_over();

  // Two calls to trigger two scheduler ticks, because sometimes the first one
  // pushes a lost update into the server, causing it to delay the clean up.
  simulator.wait(
      [&]()
        {
          return !simulator.server.has_any_activity();
        },
      config.game_service_clean_up_interval);

  EXPECT_FALSE(simulator.server.has_any_activity());
}

TEST(shut_down_test, no_new_session_after_shut_down)
{
  constexpr std::uint8_t player_count = 3;
  const bim::server::config config = bim::server::tests::new_test_config();
  bim::server::tests::client_server_simulator simulator(player_count, config);

  simulator.clients[0].authenticate();
  EXPECT_TRUE(!!simulator.clients[0].session);

  simulator.clients[1].authenticate();
  EXPECT_TRUE(!!simulator.clients[1].session);

  simulator.server.shut_down();

  simulator.clients[2].authenticate();
  EXPECT_FALSE(!!simulator.clients[2].session);
  ASSERT_TRUE(!!simulator.clients[2].authentication_error);
  EXPECT_EQ(bim::net::authentication_error_code::maintenance,
            *simulator.clients[2].authentication_error);
}

TEST(shut_down_test, no_new_game_after_shut_down)
{
  constexpr std::uint8_t player_count = 2;
  const bim::server::config config = bim::server::tests::new_test_config();
  bim::server::tests::client_server_simulator simulator(player_count, config);

  simulator.authenticate();
  simulator.server.shut_down();

  for (std::uint8_t i = 0; i != player_count; ++i)
    simulator.clients[i].new_game_auto_accept();

  simulator.wait(
      []()
        {
          return false;
        });

  EXPECT_FALSE(simulator.clients[0].started);
  EXPECT_FALSE(simulator.clients[1].started);
}
