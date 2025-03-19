// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/client_server_simulator.hpp>

#include <bim/server/tests/new_test_config.hpp>

#include <bim/server/config.hpp>

#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action_queue.hpp>
#include <bim/game/contest.hpp>

#include <gtest/gtest.h>

class player_disconnection_state : public testing::TestWithParam<int>
{
public:
  player_disconnection_state();

protected:
  const int m_ticks_for_disconnection;
  bim::server::tests::client_server_simulator m_simulator;
};

player_disconnection_state::player_disconnection_state()
  : m_ticks_for_disconnection(10)
  , m_simulator(
        GetParam(),
        [this]() -> bim::server::config
        {
          bim::server::config config = bim::server::tests::new_test_config();
          config.game_service_disconnection_lateness_threshold_in_ticks =
              m_ticks_for_disconnection;
          return config;
        }())
{}

/**
 * The players join a game, then all players except number two stop sending
 * updates. These players should be removed from the game and the game should
 * terminate.
 */
TEST_P(player_disconnection_state, players_quit)
{
  m_simulator.authenticate();
  m_simulator.join_game();

  const int player_count = GetParam();

  // Warm up, everyone is idle.
  for (std::size_t tick_index = 0;
       tick_index != 2 * bim::game::player_action_queue::queue_size;
       ++tick_index)
    m_simulator.tick();

  std::vector<bool> is_disconnected(player_count, false);

  // Player #0 stops sending updates.
  is_disconnected[0] = true;
  m_simulator.clients[0].leave_game();

  m_simulator.tick(m_ticks_for_disconnection);

  // Player #0 should be dead for all other players.
  for (int i = 0; i != player_count; ++i)
    if (!is_disconnected[i])
      {
        int active_player_count = 0;
        m_simulator.clients[i]
            .contest->registry()
            .view<bim::game::player>()
            .each(
                [this, &active_player_count,
                 i](const bim::game::player& p) -> void
                {
                  EXPECT_NE(m_simulator.clients[0].player_index, p.index)
                      << "i=" << i;
                  ++active_player_count;
                });

        EXPECT_EQ(player_count - 1, active_player_count) << "i=" << i;
      }

  // One tick to trigger the game over message from the server.
  m_simulator.tick(1);

  // If there was more than two players then the game should still be running
  // for the other players.
  if (player_count > 2)
    for (int i = 0; i != player_count; ++i)
      if (!is_disconnected[i])
        {
          EXPECT_TRUE(m_simulator.clients[i].result.still_running())
              << "i=" << i;
        }

  if (player_count >= 3)
    {
      is_disconnected[2] = true;
      m_simulator.clients[2].leave_game();

      m_simulator.tick(m_ticks_for_disconnection);

      // Player #2 should be dead for all other players.
      for (int i = 0; i != player_count; ++i)
        if (!is_disconnected[i])
          {
            int active_player_count = 0;

            m_simulator.clients[i]
                .contest->registry()
                .view<bim::game::player>()
                .each(
                    [this, &active_player_count,
                     i](const bim::game::player& p) -> void
                    {
                      EXPECT_NE(m_simulator.clients[0].player_index, p.index)
                          << "i=" << i;
                      EXPECT_NE(m_simulator.clients[2].player_index, p.index)
                          << "i=" << i;
                      ++active_player_count;
                    });

            EXPECT_EQ(player_count - 2, active_player_count) << "i=" << i;
          }

      // One tick to trigger the game over message from the server.
      m_simulator.tick(1);

      // If there was more than three players then the game should still be
      // running for the other players.
      if (player_count > 3)
        for (int i = 0; i != player_count; ++i)
          if (!is_disconnected[i])
            {
              EXPECT_TRUE(m_simulator.clients[i].result.still_running())
                  << "i=" << i;
            }
    }

  if (player_count == 4)
    {
      is_disconnected[3] = true;
      m_simulator.clients[3].leave_game();

      m_simulator.tick(m_ticks_for_disconnection);

      // Player #3 should be dead for all other players.
      for (int i = 0; i != player_count; ++i)
        if (!is_disconnected[i])
          {
            int active_player_count = 0;
            m_simulator.clients[i]
                .contest->registry()
                .view<bim::game::player>()
                .each(
                    [this, &active_player_count,
                     i](const bim::game::player& p) -> void
                    {
                      EXPECT_NE(m_simulator.clients[0].player_index, p.index)
                          << "i=" << i;
                      EXPECT_NE(m_simulator.clients[2].player_index, p.index)
                          << "i=" << i;
                      EXPECT_NE(m_simulator.clients[3].player_index, p.index)
                          << "i=" << i;
                      ++active_player_count;
                    });

            EXPECT_EQ(player_count - 3, active_player_count) << "i=" << i;
          }

      // One tick to trigger the game over message from the server.
      m_simulator.tick(1);
    }

  // At this point all players except the last one are dead. The game
  // should be over.
  for (int i = 0; i != player_count; ++i)
    if (!is_disconnected[i])
      {
        EXPECT_FALSE(m_simulator.clients[i].result.still_running())
            << "i=" << i;
        ASSERT_TRUE(m_simulator.clients[i].result.has_a_winner()) << "i=" << i;
        EXPECT_EQ(m_simulator.clients[1].player_index,
                  m_simulator.clients[i].result.winning_player())
            << "i=" << i;
      }
}

INSTANTIATE_TEST_SUITE_P(player_disconnection_state_instance,
                         player_disconnection_state, testing::Values(2, 3, 4));
