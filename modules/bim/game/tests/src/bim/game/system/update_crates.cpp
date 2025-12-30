// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_crates.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/entity_world_map.hpp>

#include <bim/game/component/burning.hpp>
#include <bim/game/component/crate.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/solid.hpp>
#include <bim/game/factory/crate.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(update_crates, burning)
{
  entt::registry registry;
  bim::game::entity_world_map entity_map(3, 3);

  const entt::entity entity_burning =
      bim::game::crate_factory(registry, entity_map, 0, 0);
  registry.emplace<bim::game::burning>(entity_burning);

  const entt::entity entity =
      bim::game::crate_factory(registry, entity_map, 0, 1);

  ASSERT_EQ(1, entity_map.entities_at(0, 0).size());
  EXPECT_EQ(entity_burning, entity_map.entities_at(0, 0)[0]);
  EXPECT_TRUE(registry.storage<bim::game::solid>().contains(entity_burning));

  ASSERT_EQ(1, entity_map.entities_at(0, 1).size());
  EXPECT_TRUE(entity == entity_map.entities_at(0, 1)[0]);
  EXPECT_TRUE(registry.storage<bim::game::solid>().contains(entity));

  bim::game::update_crates(registry, entity_map);

  EXPECT_TRUE(entity_map.entities_at(0, 0).empty());

  ASSERT_EQ(1, entity_map.entities_at(0, 1).size());
  EXPECT_TRUE(entity == entity_map.entities_at(0, 1)[0]);

  EXPECT_TRUE(registry.storage<bim::game::dead>().contains(entity_burning));
  EXPECT_FALSE(registry.storage<bim::game::dead>().contains(entity));
}
