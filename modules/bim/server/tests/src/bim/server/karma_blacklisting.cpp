// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/client_server_simulator.hpp>

#include <bim/server/tests/new_test_config.hpp>

#include <bim/server/config.hpp>

#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action_queue.hpp>
#include <bim/game/component/player_movement.hpp>
#include <bim/game/contest.hpp>

#include <gtest/gtest.h>

TEST(karma_blacklisting, blacklisted_on_short_games)
{
  bim::server::config config = bim::server::tests::new_test_config();
  config.game_service_max_duration_for_short_game = std::chrono::seconds(20);
  config.enable_karma = true;
  config.initial_karma_value = 9;
  config.disconnection_karma_adjustment = 0;
  config.short_game_karma_adjustment = -5;
  config.good_behavior_karma_adjustment = 0;
  config.karma_blacklisting_duration = std::chrono::minutes(1);
  config.karma_review_interval = std::chrono::minutes(2);

  const int player_count = 2;
  bim::server::tests::client_server_simulator simulator(player_count, config);

  const auto player_1_short_game_scenario = [&]() -> void
  {
    simulator.join_game();

    // Warm up, everyone is idle.
    for (std::size_t tick_index = 0;
         tick_index != 2 * bim::game::player_action_queue::queue_size;
         ++tick_index)
      simulator.tick();

    // One player intentionally dies.
    simulator.clients[1].set_action(bim::game::player_action{
        .movement = bim::game::player_movement::idle, .drop_bomb = true });
    simulator.tick();

    simulator.clients[1].set_action({});

    // Then wait for the bombs to to be applied, i.e. until they get out of the
    // queue.
    for (std::size_t tick_index = 0;
         tick_index != bim::game::player_action_queue::queue_size;
         ++tick_index)
      simulator.tick();

    // Wait for the game to end.
    for (int i = 0; (i != 10) && simulator.clients[0].result.still_running();
         ++i)
      simulator.tick(std::chrono::seconds(1));

    EXPECT_FALSE(simulator.clients[0].result.still_running());
  };

  simulator.authenticate();

  // Part 1, player 1 intentionally dies.
  player_1_short_game_scenario();

  // Part 2, second try.
  player_1_short_game_scenario();

  // Now the player should be blacklisted.
  simulator.clients[1].authenticate();
  EXPECT_FALSE(!!simulator.clients[1].session);
  ASSERT_TRUE(!!simulator.clients[1].authentication_error);
  EXPECT_EQ(bim::net::authentication_error_code::refused,
            *simulator.clients[1].authentication_error);

  // Wait for the karma to be reset.
  simulator.wait(std::chrono::minutes(3));

  // Now the player should be allowed.
  simulator.clients[1].authenticate();
  EXPECT_TRUE(!!simulator.clients[1].session);
}

TEST(karma_blacklisting, blacklisted_on_disconnections)
{
  bim::server::config config = bim::server::tests::new_test_config();
  config.game_service_disconnection_inactivity_delay = std::chrono::seconds(5);
  config.enable_karma = true;
  config.initial_karma_value = 9;
  config.disconnection_karma_adjustment = -5;
  config.short_game_karma_adjustment = 0;
  config.good_behavior_karma_adjustment = 0;
  config.karma_blacklisting_duration = std::chrono::minutes(1);
  config.karma_review_interval = std::chrono::minutes(2);

  const int player_count = 2;
  bim::server::tests::client_server_simulator simulator(player_count, config);

  const auto player_1_disconnection_scenario = [&]() -> void
  {
    simulator.join_game();

    // Warm up, everyone is idle.
    for (std::size_t tick_index = 0;
         tick_index != 2 * bim::game::player_action_queue::queue_size;
         ++tick_index)
      simulator.tick();

    // One player leaves.
    simulator.clients[1].leave_game();

    // Then wait for the automatic disconnection.
    simulator.tick(std::chrono::seconds(6));

    EXPECT_FALSE(simulator.clients[0].result.still_running());
    ASSERT_TRUE(simulator.clients[0].result.has_a_winner());
    EXPECT_EQ(simulator.clients[0].player_index,
              simulator.clients[0].result.winning_player());
  };

  simulator.authenticate();

  // Part 1, player 1 intentionally dies.
  player_1_disconnection_scenario();

  // Part 2, second try.
  simulator.clients[1].authenticate();
  player_1_disconnection_scenario();

  // Now the player should be blacklisted.
  simulator.clients[1].authenticate();
  EXPECT_FALSE(!!simulator.clients[1].session);
  ASSERT_TRUE(!!simulator.clients[1].authentication_error);
  EXPECT_EQ(bim::net::authentication_error_code::refused,
            *simulator.clients[1].authentication_error);

  // Wait for the karma to be reset.
  simulator.wait(std::chrono::minutes(3));

  // Now the player should be allowed.
  simulator.clients[1].authenticate();
  EXPECT_TRUE(!!simulator.clients[1].session);
}
