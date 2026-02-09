// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/client_server_simulator.hpp>

#include <bim/server/tests/new_test_config.hpp>

#include <bim/game/component/player_action_queue.hpp>
#include <bim/game/component/player_movement.hpp>
#include <bim/game/player_action.hpp>

#include <gtest/gtest.h>

class game_reward_test : public testing::TestWithParam<int>
{
public:
  game_reward_test();

protected:
  void drop_bombs_wait_game_over(int player_count, int surviving_player);

protected:
  bim::server::tests::client_server_simulator m_simulator;
};

game_reward_test::game_reward_test()
  : m_simulator(GetParam(),
                []()
                {
                  bim::server::config config =
                      bim::server::tests::new_test_config();

                  config.game_service_max_duration_for_short_game =
                      std::chrono::seconds(10);
                  config.game_service_disconnection_inactivity_delay =
                      std::chrono::seconds(50);

                  config.game_service_coins_per_victory = 100;
                  config.game_service_coins_per_defeat = 200;
                  config.game_service_coins_per_draw = 300;
                  config.game_service_coins_per_short_game_victory = 10;
                  config.game_service_coins_per_short_game_defeat = 20;
                  config.game_service_coins_per_short_game_draw = 30;
                  return config;
                }())
{}

void game_reward_test::drop_bombs_wait_game_over(int player_count,
                                                 int surviving_player)
{
  // Everyone except the last player drop a bomb.
  for (int i = 0; i != player_count; ++i)
    if (i == surviving_player)
      m_simulator.clients[i].set_action(bim::game::player_action{
          .movement = bim::game::player_movement::idle, .drop_bomb = false });
    else
      m_simulator.clients[i].set_action(bim::game::player_action{
          .movement = bim::game::player_movement::idle, .drop_bomb = true });

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

  // Wait for the game to end.
  const auto still_running = [this, player_count]() -> bool
  {
    for (int i = 0; i != player_count; ++i)
      if (m_simulator.clients[i].result.still_running())
        return true;

    return false;
  };

  for (int i = 0; (i != 10) && still_running(); ++i)
    m_simulator.tick(std::chrono::seconds(1));
}

TEST_P(game_reward_test, winner_short_game)
{
  m_simulator.authenticate();
  m_simulator.join_game();

  const int player_count = GetParam();
  const int last_player = player_count - 1;
  ASSERT_LE(0, last_player);

  drop_bombs_wait_game_over(player_count, last_player);

  // The game is over, check the rewards.
  for (int i = 0; i != player_count; ++i)
    if (i == last_player)
      EXPECT_EQ(m_simulator.config.game_service_coins_per_short_game_victory,
                m_simulator.clients[i].coins_reward);
    else
      EXPECT_EQ(m_simulator.config.game_service_coins_per_short_game_defeat,
                m_simulator.clients[i].coins_reward);
}

TEST_P(game_reward_test, draw_short_game)
{
  m_simulator.authenticate();
  m_simulator.join_game();

  const int player_count = GetParam();

  drop_bombs_wait_game_over(player_count, -1);

  // The game is over, check the rewards.
  for (int i = 0; i != player_count; ++i)
    EXPECT_EQ(m_simulator.config.game_service_coins_per_short_game_draw,
              m_simulator.clients[i].coins_reward);
}

TEST_P(game_reward_test, winner_long_game)
{
  m_simulator.authenticate();
  m_simulator.join_game();

  const int player_count = GetParam();
  const int last_player = player_count - 1;
  ASSERT_LE(0, last_player);

  m_simulator.tick(
      m_simulator.config.game_service_max_duration_for_short_game);
  drop_bombs_wait_game_over(player_count, last_player);

  // The game is over, check the rewards.
  for (int i = 0; i != player_count; ++i)
    if (i == last_player)
      EXPECT_EQ(m_simulator.config.game_service_coins_per_victory,
                m_simulator.clients[i].coins_reward);
    else
      EXPECT_EQ(m_simulator.config.game_service_coins_per_defeat,
                m_simulator.clients[i].coins_reward);
}

TEST_P(game_reward_test, draw_long_game)
{
  m_simulator.authenticate();
  m_simulator.join_game();

  const int player_count = GetParam();

  m_simulator.tick(
      m_simulator.config.game_service_max_duration_for_short_game);
  drop_bombs_wait_game_over(player_count, -1);

  // The game is over, check the rewards.
  for (int i = 0; i != player_count; ++i)
    EXPECT_EQ(m_simulator.config.game_service_coins_per_draw,
              m_simulator.clients[i].coins_reward);
}

INSTANTIATE_TEST_SUITE_P(game_reward_test_instance, game_reward_test,
                         testing::Values(2, 3, 4));
