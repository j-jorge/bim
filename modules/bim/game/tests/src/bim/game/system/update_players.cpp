// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_players.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/bomb.hpp>
#include <bim/game/component/burning.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/flame_direction.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/factory/flame.hpp>
#include <bim/game/factory/player.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(bim_game_update_players, death_on_flame_collision)
{
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

  const entt::entity player_on_flame =
      bim::game::player_factory(registry, 0, 1, 0);
  const entt::entity other_player_on_flame =
      bim::game::player_factory(registry, 1, 1, 1);
  const entt::entity surviving_player =
      bim::game::player_factory(registry, 2, 0, 1);

  bim::game::update_players(registry, arena);

  EXPECT_TRUE(registry.storage<bim::game::dead>().contains(player_on_flame));
  EXPECT_TRUE(
      registry.storage<bim::game::dead>().contains(other_player_on_flame));
  EXPECT_FALSE(registry.storage<bim::game::dead>().contains(surviving_player));
}
