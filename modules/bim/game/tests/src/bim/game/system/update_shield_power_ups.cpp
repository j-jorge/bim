// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_shield_power_ups.hpp>

#include <bim/game/entity_world_map.hpp>

#include <bim/game/component/dead.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/shield.hpp>
#include <bim/game/factory/player.hpp>
#include <bim/game/factory/power_up.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

namespace bim::game
{
  class shield_power_up;
}

TEST(update_shield_power_ups, player_collision)
{
  entt::registry registry;
  bim::game::entity_world_map entity_map(3, 3);
  const int x = 1;
  const int y = 1;

  const entt::entity power_up_entity =
      power_up_factory<bim::game::shield_power_up>(registry, entity_map, x, y);
  const entt::entity player_entity = bim::game::player_factory(
      registry, entity_map, 0, x, y, bim::game::animation_id{});

  bim::game::update_shield_power_ups(registry, entity_map);

  EXPECT_TRUE(registry.storage<bim::game::shield>().contains(player_entity));
  EXPECT_TRUE(registry.storage<bim::game::dead>().contains(power_up_entity));
  ASSERT_EQ(1, entity_map.entities_at(x, y).size());
  EXPECT_EQ(player_entity, entity_map.entities_at(x, y)[0]);
}

TEST(update_shield_power_ups, two_players_only_one_gets_the_power_up)
{
  entt::registry registry;
  bim::game::entity_world_map entity_map(3, 3);
  const int x = 1;
  const int y = 1;

  power_up_factory<bim::game::shield_power_up>(registry, entity_map, x, y);

  const entt::entity player_entity[2] = {
    bim::game::player_factory(registry, entity_map, 0, x, y,
                              bim::game::animation_id{}),
    bim::game::player_factory(registry, entity_map, 1, x, y,
                              bim::game::animation_id{})
  };

  EXPECT_FALSE(
      registry.storage<bim::game::shield>().contains(player_entity[0]));
  EXPECT_FALSE(
      registry.storage<bim::game::shield>().contains(player_entity[1]));

  bim::game::update_shield_power_ups(registry, entity_map);

  EXPECT_NE(registry.storage<bim::game::shield>().contains(player_entity[0]),
            registry.storage<bim::game::shield>().contains(player_entity[1]));
}
