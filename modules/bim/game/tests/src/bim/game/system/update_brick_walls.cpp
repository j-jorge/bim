// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_brick_walls.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/brick_wall.hpp>
#include <bim/game/component/burning.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/factory/brick_wall.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(update_brick_walls, burning)
{
  entt::registry registry;
  bim::game::arena arena(3, 3);

  const entt::entity entity_burning =
      bim::game::brick_wall_factory(registry, arena, 0, 0);
  registry.emplace<bim::game::burning>(entity_burning);

  const entt::entity entity =
      bim::game::brick_wall_factory(registry, arena, 0, 1);

  EXPECT_TRUE(entity_burning == arena.entity_at(0, 0));
  EXPECT_TRUE(entity == arena.entity_at(0, 1));

  bim::game::update_brick_walls(registry, arena);

  EXPECT_TRUE(entt::null == arena.entity_at(0, 0));
  EXPECT_TRUE(entity == arena.entity_at(0, 1));
  EXPECT_TRUE(registry.storage<bim::game::dead>().contains(entity_burning));
  EXPECT_FALSE(registry.storage<bim::game::dead>().contains(entity));
}
