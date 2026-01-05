// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_players.hpp>

#include <bim/game/entity_world_map.hpp>

#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/bomb.hpp>
#include <bim/game/component/burning.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/flame_direction.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/fill_context.hpp>
#include <bim/game/context/player_animations.hpp>
#include <bim/game/factory/flame.hpp>
#include <bim/game/factory/player.hpp>
#include <bim/game/system/update_flames.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(bim_game_update_players, death_on_flame_collision)
{
  bim::game::context context;
  bim::game::fill_context(context);

  entt::registry registry;
  const int arena_width = 3;
  const int arena_height = 3;
  bim::game::entity_world_map entity_map(arena_width, arena_height);

  bim::game::flame_factory(registry, 1, 0, bim::game::flame_direction::up,
                           bim::game::flame_segment::tip);
  bim::game::flame_factory(registry, 1, 1, bim::game::flame_direction::up,
                           bim::game::flame_segment::tip);

  const bim::game::player_animations& player_animations =
      context.get<const bim::game::player_animations>();

  const bim::game::animation_state& player_on_flame =
      registry.storage<bim::game::animation_state>().get(
          bim::game::player_factory(registry, entity_map, 0, 1, 0,
                                    player_animations.idle_down));
  const bim::game::animation_state& other_player_on_flame =
      registry.storage<bim::game::animation_state>().get(
          bim::game::player_factory(registry, entity_map, 1, 1, 1,
                                    player_animations.idle_down));
  const bim::game::animation_state& surviving_player =
      registry.storage<bim::game::animation_state>().get(
          bim::game::player_factory(registry, entity_map, 2, 0, 1,
                                    player_animations.idle_down));

  bim::game::update_flames(registry, entity_map);
  bim::game::update_players(context, registry);

  EXPECT_EQ(player_animations.burn, player_on_flame.model);
  EXPECT_EQ(player_animations.burn, other_player_on_flame.model);
  EXPECT_EQ(player_animations.idle_down, surviving_player.model);
}

TEST(bim_game_update_players, death_of_multiple_players_on_same_flame)
{
  bim::game::context context;
  bim::game::fill_context(context);

  entt::registry registry;
  const int arena_width = 3;
  const int arena_height = 3;
  bim::game::entity_world_map entity_map(arena_width, arena_height);

  bim::game::flame_factory(registry, 1, 0, bim::game::flame_direction::up,
                           bim::game::flame_segment::tip);
  bim::game::flame_factory(registry, 1, 1, bim::game::flame_direction::up,
                           bim::game::flame_segment::tip);
  bim::game::flame_factory(registry, 1, 2, bim::game::flame_direction::up,
                           bim::game::flame_segment::tip);

  const bim::game::player_animations& player_animations =
      context.get<const bim::game::player_animations>();

  const bim::game::animation_state* const player_state[4] = {
    &registry.storage<bim::game::animation_state>().get(
        bim::game::player_factory(registry, entity_map, 0, 1, 0,
                                  player_animations.idle_down)),
    &registry.storage<bim::game::animation_state>().get(
        bim::game::player_factory(registry, entity_map, 1, 1, 1,
                                  player_animations.idle_down)),
    &registry.storage<bim::game::animation_state>().get(
        bim::game::player_factory(registry, entity_map, 2, 1, 2,
                                  player_animations.idle_down)),
    &registry.storage<bim::game::animation_state>().get(
        bim::game::player_factory(registry, entity_map, 3, 1, 2,
                                  player_animations.idle_down))
  };

  bim::game::update_flames(registry, entity_map);
  bim::game::update_players(context, registry);

  EXPECT_EQ(player_animations.burn, player_state[0]->model);
  EXPECT_EQ(player_animations.burn, player_state[1]->model);
  EXPECT_EQ(player_animations.burn, player_state[2]->model);
  EXPECT_EQ(player_animations.burn, player_state[3]->model);
}
