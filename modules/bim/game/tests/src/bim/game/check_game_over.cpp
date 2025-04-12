// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/check_game_over.hpp>

#include <bim/game/context/context.hpp>
#include <bim/game/context/fill_context.hpp>
#include <bim/game/factory/main_timer.hpp>
#include <bim/game/factory/player.hpp>
#include <bim/game/system/update_timers.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(bim_game_check_game_over, no_player)
{
  bim::game::context context;
  bim::game::fill_context(context);

  entt::registry registry;

  const bim::game::contest_result result =
      bim::game::check_game_over(context, registry);

  EXPECT_FALSE(result.still_running());
  EXPECT_FALSE(result.has_a_winner());
}

TEST(bim_game_check_game_over, one_player)
{
  bim::game::context context;
  bim::game::fill_context(context);

  entt::registry registry;
  bim::game::player_factory(registry, 2, 0, 0, bim::game::animation_id{});

  const bim::game::contest_result result =
      bim::game::check_game_over(context, registry);

  EXPECT_FALSE(result.still_running());
  ASSERT_TRUE(result.has_a_winner());
  EXPECT_EQ(2, result.winning_player());
}

TEST(bim_game_check_game_over, two_players)
{
  bim::game::context context;
  bim::game::fill_context(context);

  entt::registry registry;
  bim::game::player_factory(registry, 1, 0, 0, bim::game::animation_id{});
  bim::game::player_factory(registry, 2, 0, 0, bim::game::animation_id{});

  const bim::game::contest_result result =
      bim::game::check_game_over(context, registry);

  EXPECT_TRUE(result.still_running());
  EXPECT_FALSE(result.has_a_winner());
}

TEST(bim_game_check_game_over, three_players)
{
  bim::game::context context;
  bim::game::fill_context(context);

  entt::registry registry;
  bim::game::player_factory(registry, 0, 0, 0, bim::game::animation_id{});
  bim::game::player_factory(registry, 1, 0, 0, bim::game::animation_id{});
  bim::game::player_factory(registry, 2, 0, 0, bim::game::animation_id{});

  const bim::game::contest_result result =
      bim::game::check_game_over(context, registry);

  EXPECT_TRUE(result.still_running());
  EXPECT_FALSE(result.has_a_winner());
}

TEST(bim_game_check_game_over, four_players)
{
  bim::game::context context;
  bim::game::fill_context(context);

  entt::registry registry;
  bim::game::player_factory(registry, 0, 0, 0, bim::game::animation_id{});
  bim::game::player_factory(registry, 1, 0, 0, bim::game::animation_id{});
  bim::game::player_factory(registry, 2, 0, 0, bim::game::animation_id{});
  bim::game::player_factory(registry, 3, 0, 0, bim::game::animation_id{});

  const bim::game::contest_result result =
      bim::game::check_game_over(context, registry);

  EXPECT_TRUE(result.still_running());
  EXPECT_FALSE(result.has_a_winner());
}

TEST(bim_game_check_game_over, two_players_timeout)
{
  bim::game::context context;
  bim::game::fill_context(context);

  entt::registry registry;
  bim::game::player_factory(registry, 1, 0, 0, bim::game::animation_id{});
  bim::game::player_factory(registry, 2, 0, 0, bim::game::animation_id{});
  bim::game::main_timer_factory(registry, std::chrono::seconds(1));

  bim::game::contest_result result =
      bim::game::check_game_over(context, registry);

  EXPECT_TRUE(result.still_running());
  EXPECT_FALSE(result.has_a_winner());

  bim::game::update_timers(registry, std::chrono::seconds(1));

  result = bim::game::check_game_over(context, registry);

  EXPECT_FALSE(result.still_running());
  EXPECT_FALSE(result.has_a_winner());
}

TEST(bim_game_check_game_over, three_players_timeout)
{
  bim::game::context context;
  bim::game::fill_context(context);

  entt::registry registry;
  bim::game::player_factory(registry, 0, 0, 0, bim::game::animation_id{});
  bim::game::player_factory(registry, 1, 0, 0, bim::game::animation_id{});
  bim::game::player_factory(registry, 2, 0, 0, bim::game::animation_id{});
  bim::game::main_timer_factory(registry, std::chrono::seconds(1));

  bim::game::contest_result result =
      bim::game::check_game_over(context, registry);

  EXPECT_TRUE(result.still_running());
  EXPECT_FALSE(result.has_a_winner());

  bim::game::update_timers(registry, std::chrono::seconds(1));

  result = bim::game::check_game_over(context, registry);

  EXPECT_FALSE(result.still_running());
  EXPECT_FALSE(result.has_a_winner());
}

TEST(bim_game_check_game_over, four_players_timeout)
{
  bim::game::context context;
  bim::game::fill_context(context);

  entt::registry registry;
  bim::game::player_factory(registry, 0, 0, 0, bim::game::animation_id{});
  bim::game::player_factory(registry, 1, 0, 0, bim::game::animation_id{});
  bim::game::player_factory(registry, 2, 0, 0, bim::game::animation_id{});
  bim::game::player_factory(registry, 3, 0, 0, bim::game::animation_id{});
  bim::game::main_timer_factory(registry, std::chrono::seconds(1));

  bim::game::contest_result result =
      bim::game::check_game_over(context, registry);

  EXPECT_TRUE(result.still_running());
  EXPECT_FALSE(result.has_a_winner());

  bim::game::update_timers(registry, std::chrono::seconds(1));

  result = bim::game::check_game_over(context, registry);

  EXPECT_FALSE(result.still_running());
  EXPECT_FALSE(result.has_a_winner());
}
