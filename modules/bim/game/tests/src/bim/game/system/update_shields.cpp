// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_shields.hpp>

#include <bim/game/arena.hpp>

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
  bim::game::arena arena(3, 3);

  const auto insert_flame = [&](int x, int y)
  {
    arena.put_entity(x, y,
                     bim::game::flame_factory(registry, x, y,
                                              bim::game::flame_direction::up,
                                              bim::game::flame_segment::tip));
  };

  insert_flame(1, 0);

  const bim::game::player_animations& player_animations =
      context.get<const bim::game::player_animations>();

  const entt::entity player_entity = bim::game::player_factory(
      registry, 0, 1, 0, player_animations.idle_down);
  registry.emplace<bim::game::shield>(player_entity);

  const bim::game::animation_state& animations =
      registry.storage<bim::game::animation_state>().get(player_entity);

  EXPECT_EQ(player_animations.idle_down, animations.model);
  EXPECT_TRUE(bim::game::has_shield(registry, player_entity));

  bim::game::update_flames(registry, arena);
  EXPECT_TRUE(registry.storage<bim::game::burning>().contains(player_entity));
  EXPECT_TRUE(bim::game::has_shield(registry, player_entity));

  bim::game::update_shields(registry);
  EXPECT_FALSE(registry.storage<bim::game::burning>().contains(player_entity));
  EXPECT_FALSE(bim::game::has_shield(registry, player_entity));

  bim::game::update_players(context, registry);
  EXPECT_EQ(player_animations.idle_down, animations.model);
}
