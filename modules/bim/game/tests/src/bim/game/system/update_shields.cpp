// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_shields.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/entity_world_map.hpp>

#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/bomb.hpp>
#include <bim/game/component/burning.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/flame_direction.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/shield.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/fill_context.hpp>
#include <bim/game/context/player_animations.hpp>
#include <bim/game/factory/flame.hpp>
#include <bim/game/factory/player.hpp>
#include <bim/game/system/update_flames.hpp>
#include <bim/game/system/update_players.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(bim_game_update_shields, save_player_from_first_hit)
{
  bim::game::context context;
  bim::game::fill_context(context);

  entt::registry registry;
  const int arena_width = 3;
  const int arena_height = 3;
  bim::game::entity_world_map entity_map(arena_width, arena_height);

  const int x = 0;
  const int y = 1;

  // We have one flame, that is going to burn the player.
  bim::game::flame_factory(registry, x, y, bim::game::flame_direction::up,
                           bim::game::flame_segment::tip);

  const bim::game::player_animations& player_animations =
      context.get<const bim::game::player_animations>();

  // We place the player where the flame is.
  const entt::entity player_entity = bim::game::player_factory(
      registry, entity_map, 0, x, y, player_animations.idle_down);
  // And we give a shield to the player.
  registry.emplace<bim::game::shield>(player_entity);

  const bim::game::animation_state& animations =
      registry.storage<bim::game::animation_state>().get(player_entity);

  EXPECT_EQ(player_animations.idle_down, animations.model);
  EXPECT_TRUE(bim::game::has_shield(registry, player_entity));

  // We update the flames. The player is going to be hit and should lose its
  // shield.
  bim::game::update_flames(registry, entity_map);
  // Player is in a flame -> player is burning.
  EXPECT_TRUE(registry.storage<bim::game::burning>().contains(player_entity));
  EXPECT_TRUE(bim::game::has_shield(registry, player_entity));

  // We update the shield. The burning player loses its shield and stops
  // burning.
  bim::game::update_shields(registry);
  EXPECT_FALSE(registry.storage<bim::game::burning>().contains(player_entity));
  EXPECT_FALSE(bim::game::has_shield(registry, player_entity));

  // Confirm that the player is still here.
  bim::game::update_players(context, registry);
  EXPECT_EQ(player_animations.idle_down, animations.model);
}
