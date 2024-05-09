#include <bim/game/system/update_flame_power_ups.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/dead.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/factory/flame_power_up.hpp>
#include <bim/game/factory/player.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(update_flame_power_ups, player_collision)
{
  entt::registry registry;
  bim::game::arena arena(3, 3);
  const int x = 1;
  const int y = 1;

  const entt::entity power_up_entity =
      flame_power_up_factory(registry, arena, x, y);
  const entt::entity player_entity =
      bim::game::player_factory(registry, 0, x, y);

  const bim::game::player& player =
      registry.get<bim::game::player>(player_entity);

  const int bomb_strength = player.bomb_strength;

  bim::game::update_flame_power_ups(registry, arena);

  EXPECT_EQ(bomb_strength + 1, player.bomb_strength);
  EXPECT_TRUE(registry.storage<bim::game::dead>().contains(power_up_entity));
}
