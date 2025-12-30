// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/entity_world_map.hpp>

#include <bim/game/component/position_on_grid.hpp>
#include <entt/entity/registry.hpp>

#include <algorithm>

#include <gtest/gtest.h>

TEST(bim_game_entity_world_map, update)
{
  entt::registry registry;

  const entt::entity entities[] = { registry.create(), registry.create(),
                                    registry.create(), registry.create(),
                                    registry.create(), registry.create() };

  bim::game::entity_world_map map(2, 2);

  map.put_entity(entities[0], 0, 0);
  map.put_entity(entities[1], 1, 0);
  map.put_entity(entities[2], 0, 1);
  map.put_entity(entities[3], 1, 0);
  map.put_entity(entities[4], 1, 1);
  map.put_entity(entities[5], 1, 0);

  ASSERT_EQ(1, map.entities_at(0, 0).size());
  EXPECT_EQ(entities[0], map.entities_at(0, 0)[0]);

  ASSERT_EQ(1, map.entities_at(0, 1).size());
  EXPECT_EQ(entities[2], map.entities_at(0, 1)[0]);

  ASSERT_EQ(1, map.entities_at(1, 1).size());
  EXPECT_EQ(entities[4], map.entities_at(1, 1)[0]);

  const std::span<const entt::entity> span = map.entities_at(1, 0);
  ASSERT_EQ(3, span.size());
  EXPECT_NE(span.end(), std::ranges::find(span, entities[1]));
  EXPECT_NE(span.end(), std::ranges::find(span, entities[3]));
  EXPECT_NE(span.end(), std::ranges::find(span, entities[5]));
}

TEST(bim_game_entity_world_map, erase)
{
  entt::registry registry;

  const entt::entity entities[] = { registry.create(), registry.create(),
                                    registry.create(), registry.create(),
                                    registry.create(), registry.create() };

  bim::game::entity_world_map map(2, 2);

  map.put_entity(entities[0], 0, 0);
  map.put_entity(entities[1], 1, 0);
  map.put_entity(entities[2], 0, 1);
  map.put_entity(entities[3], 1, 0);
  map.put_entity(entities[4], 1, 1);
  map.put_entity(entities[5], 1, 0);

  map.erase_entity(entities[2], 0, 1);
  map.erase_entity(entities[3], 1, 0);

  ASSERT_EQ(1, map.entities_at(0, 0).size());
  EXPECT_EQ(entities[0], map.entities_at(0, 0)[0]);

  EXPECT_EQ(0, map.entities_at(0, 1).size());

  ASSERT_EQ(1, map.entities_at(1, 1).size());
  EXPECT_EQ(entities[4], map.entities_at(1, 1)[0]);

  const std::span<const entt::entity> span = map.entities_at(1, 0);
  ASSERT_EQ(2, span.size());
  EXPECT_NE(span.end(), std::ranges::find(span, entities[1]));
  EXPECT_NE(span.end(), std::ranges::find(span, entities[5]));
}
