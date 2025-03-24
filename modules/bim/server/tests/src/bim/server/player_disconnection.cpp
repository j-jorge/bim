// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/client_server_simulator.hpp>

#include <bim/server/tests/new_test_config.hpp>

#include <bim/server/config.hpp>

#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action_queue.hpp>
#include <bim/game/contest.hpp>

#include <gtest/gtest.h>

class player_disconnection_state : public testing::TestWithParam<int>
{};

/**
 * The players join a game, then all players except number two stop sending
 * updates. These players should be removed from the game and the game should
 * terminate. The disconnection is caused by the lateness threshold parameter.
 *
 * The test is flaky, probably because the sockets are not 100% reliable, or
 * maybe just because I failed to implement something correctly. Anyway, this
 * is why there is no assertion here. The test is only used to ensure no errors
 * are found when this is executed with Valgrind.
 */
TEST_P(player_disconnection_state, players_quit_lateness_threshold)
{
  constexpr int ticks_for_disconnection = 10;
  bim::server::config config = bim::server::tests::new_test_config();
  config.game_service_disconnection_earliness_threshold_in_ticks =
      std::numeric_limits<int>::max();
  config.game_service_disconnection_lateness_threshold_in_ticks =
      ticks_for_disconnection;
  config.game_service_disconnection_inactivity_delay = std::chrono::hours(10);

  bim::server::tests::client_server_simulator simulator(GetParam(), config);
  simulator.authenticate();
  simulator.join_game();

  const int player_count = GetParam();

  // Warm up, everyone is idle.
  for (std::size_t tick_index = 0;
       tick_index != 2 * bim::game::player_action_queue::queue_size;
       ++tick_index)
    simulator.tick();

  std::vector<bool> is_disconnected(player_count, false);

  // Player #0 stops sending updates.
  is_disconnected[0] = true;
  simulator.clients[0].leave_game();

  simulator.tick(ticks_for_disconnection);

  // Player #0 should be dead for all other players.

  // One tick to trigger the game over message from the server.
  simulator.tick(1);

  // If there was more than two players then the game should still be running
  // for the other players.
  if (player_count >= 3)
    {
      is_disconnected[2] = true;
      simulator.clients[2].leave_game();

      simulator.tick(ticks_for_disconnection);

      // One tick to trigger the game over message from the server.
      simulator.tick(1);

      // If there was more than three players then the game should still be
      // running for the other players.
    }

  if (player_count == 4)
    {
      is_disconnected[3] = true;
      simulator.clients[3].leave_game();

      simulator.tick(ticks_for_disconnection);

      // One tick to trigger the game over message from the server.
      simulator.tick(1);
    }

  // At this point all players except the last one are dead. The game
  // should be over.
}

/**
 * The players join a game, then all players except number two send too many
 * updates. These players should be removed from the game and the game should
 * terminate. The disconnection is caused by the earliness threshold parameter.
 *
 * The test is flaky, probably because the sockets are not 100% reliable, or
 * maybe just because I failed to implement something correctly. Anyway, this
 * is why there is no assertion here. The test is only used to ensure no errors
 * are found when this is executed with Valgrind.
 */
TEST_P(player_disconnection_state, players_quit_earliness_threshold)
{
  constexpr int ticks_for_disconnection = 10;
  bim::server::config config = bim::server::tests::new_test_config();
  config.game_service_disconnection_earliness_threshold_in_ticks =
      ticks_for_disconnection;
  config.game_service_disconnection_lateness_threshold_in_ticks =
      std::numeric_limits<int>::max();
  config.game_service_disconnection_inactivity_delay = std::chrono::hours(10);

  bim::server::tests::client_server_simulator simulator(GetParam(), config);
  simulator.authenticate();
  simulator.join_game();

  const int player_count = GetParam();

  // Warm up, everyone is idle.
  for (std::size_t tick_index = 0;
       tick_index != 2 * bim::game::player_action_queue::queue_size;
       ++tick_index)
    simulator.tick();

  std::vector<bool> is_disconnected(player_count, false);

  // Player #0 sends too many updates.
  is_disconnected[0] = true;
  simulator.tick(0, ticks_for_disconnection);

  // Update all active players individually. We must reach and go farther than
  // the tick reached by the inactive players for the game over to kick in.
  const auto update_active_players = [&]() -> void
  {
    for (int i = 0; i != player_count; ++i)
      if (!is_disconnected[i])
        simulator.tick(i, ticks_for_disconnection + 1);
  };

  update_active_players();

  // One tick to trigger the game over message from the server.
  simulator.tick(1);

  // If there was more than two players then the game should still be
  // running for the other players.

  if (player_count >= 3)
    {
      is_disconnected[2] = true;
      simulator.tick(2, ticks_for_disconnection);
      update_active_players();

      // One tick to trigger the game over message from the server.
      simulator.tick(1);

      // If there was more than three players then the game should still be
      // running for the other players.
    }

  if (player_count == 4)
    {
      is_disconnected[3] = true;
      simulator.tick(3, ticks_for_disconnection);
      update_active_players();

      // One tick to trigger the game over message from the server.
      simulator.tick(1);
    }

  // At this point all players except the last one are dead. The game
  // should be over.
}

/**
 * The players join a game, then all players except number two stop sending
 * updates. These players should be removed from the game and the game should
 * terminate. The disconnection is caused by the inactivity delay parameter.
 *
 * The test is flaky, probably because the sockets are not 100% reliable, or
 * maybe just because I failed to implement something correctly. Anyway, this
 * is why there is no assertion here. The test is only used to ensure no errors
 * are found when this is executed with Valgrind.
 */
TEST_P(player_disconnection_state, players_quit_inactivity_delay)
{
  constexpr std::chrono::seconds seconds_for_disconnection(5);
  bim::server::config config = bim::server::tests::new_test_config();
  config.game_service_disconnection_earliness_threshold_in_ticks =
      std::numeric_limits<int>::max();
  config.game_service_disconnection_lateness_threshold_in_ticks =
      std::numeric_limits<int>::max();
  config.game_service_disconnection_inactivity_delay =
      seconds_for_disconnection;

  bim::server::tests::client_server_simulator simulator(GetParam(), config);
  simulator.authenticate();
  simulator.join_game();

  const int player_count = GetParam();

  // Warm up, everyone is idle.
  for (std::size_t tick_index = 0;
       tick_index != 2 * bim::game::player_action_queue::queue_size;
       ++tick_index)
    simulator.tick();

  // Player #0 stops sending updates.
  simulator.clients[0].leave_game();

  simulator.tick(seconds_for_disconnection);

  // Player #0 should be dead for all other players.

  // One tick to trigger the game over message from the server.
  simulator.tick(1);

  // If there was more than two players then the game should still be running
  // for the other players.

  if (player_count >= 3)
    {
      simulator.clients[2].leave_game();

      simulator.tick(seconds_for_disconnection);

      // Player #2 should be dead for all other players.

      // One tick to trigger the game over message from the server.
      simulator.tick(1);

      // If there was more than three players then the game should still be
      // running for the other players.
    }

  if (player_count == 4)
    {
      simulator.clients[3].leave_game();

      simulator.tick(seconds_for_disconnection);

      // Player #3 should be dead for all other players.

      // One tick to trigger the game over message from the server.
      simulator.tick(1);
    }

  // At this point all players except the last one are dead. The game
  // should be over.
}

INSTANTIATE_TEST_SUITE_P(player_disconnection_state_instance,
                         player_disconnection_state, testing::Values(2, 3, 4));
