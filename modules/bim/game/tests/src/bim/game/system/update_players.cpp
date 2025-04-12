// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_players.hpp>

#include <bim/game/arena.hpp>

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

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(bim_game_update_players, death_on_flame_collision)
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
  insert_flame(1, 1);

  const bim::game::player_animations& player_animations =
      context.get<const bim::game::player_animations>();

  const bim::game::animation_state& player_on_flame =
      registry.storage<bim::game::animation_state>().get(
          bim::game::player_factory(registry, 0, 1, 0,
                                    player_animations.idle_down));
  const bim::game::animation_state& other_player_on_flame =
      registry.storage<bim::game::animation_state>().get(
          bim::game::player_factory(registry, 1, 1, 1,
                                    player_animations.idle_down));
  const bim::game::animation_state& surviving_player =
      registry.storage<bim::game::animation_state>().get(
          bim::game::player_factory(registry, 2, 0, 1,
                                    player_animations.idle_down));

  bim::game::update_players(context, registry, arena);

  EXPECT_EQ(player_animations.burn, player_on_flame.model);
  EXPECT_EQ(player_animations.burn, other_player_on_flame.model);
  EXPECT_EQ(player_animations.idle_down, surviving_player.model);
}
