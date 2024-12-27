// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/client_server_simulator.hpp>

#include <bim/server/config.hpp>

#include <bim/game/component/player_action_queue.hpp>
#include <bim/game/component/player_movement.hpp>
#include <bim/game/player_action.hpp>

#include <gtest/gtest.h>

class new_game_after_game_over_test : public testing::TestWithParam<int>
{
public:
  new_game_after_game_over_test();

protected:
  bim::server::tests::client_server_simulator m_simulator;
};

new_game_after_game_over_test::new_game_after_game_over_test()
  : m_simulator(GetParam(), bim::server::config(10005))
{}

/**
 * The players join a game, then when the game is over they all ask for another
 * game.
 */
TEST_P(new_game_after_game_over_test, game_over_then_new_random_game)
{
  m_simulator.authenticate();
  m_simulator.join_game();

  const int player_count = GetParam();
  const int last_player = player_count - 1;
  ASSERT_LE(0, last_player);

  // Everyone except the last player drop a bomb.
  for (int i = 0; i != last_player; ++i)
    m_simulator.clients[i].set_action(bim::game::player_action{
        .movement = bim::game::player_movement::idle, .drop_bomb = true });

  m_simulator.clients[last_player].set_action(bim::game::player_action{
      .movement = bim::game::player_movement::idle, .drop_bomb = false });

  m_simulator.tick();

  // Then wait for the bombs to to be applied, i.e. until they get out of the
  // queue.
  for (std::size_t tick_index = 0;
       tick_index != bim::game::player_action_queue::queue_size; ++tick_index)
    {
      for (int i = 0; i != player_count; ++i)
        m_simulator.clients[i].set_action({});

      m_simulator.tick();
    }

  // Wait three game-seconds for the bomb to explode.
  m_simulator.tick(std::chrono::seconds(3));
  // Keep running a bit to synchronize with the server.
  m_simulator.tick(std::chrono::seconds(1));

  // At this point all players except the last one are dead. The game should be
  // over.
  for (int i = 0; i < player_count; ++i)
    {
      EXPECT_FALSE(m_simulator.clients[i].result.still_running()) << "i=" << i;
      ASSERT_TRUE(m_simulator.clients[i].result.has_a_winner()) << "i=" << i;
      EXPECT_EQ(m_simulator.clients[last_player].player_index,
                m_simulator.clients[i].result.winning_player())
          << "i=" << i;
    }

  // The game is over, it's time for a new game.
  m_simulator.join_game();

  for (int i = 0; i != player_count; ++i)
    {
      ASSERT_TRUE(!!m_simulator.clients[i].started) << "i=" << i;
      EXPECT_TRUE(*m_simulator.clients[i].started) << "i=" << i;
    }
}

INSTANTIATE_TEST_SUITE_P(new_game_after_game_over_test_instance,
                         new_game_after_game_over_test,
                         testing::Values(2, 3, 4));
