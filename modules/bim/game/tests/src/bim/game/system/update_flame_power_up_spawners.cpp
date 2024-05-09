#include <bim/game/system/update_flame_power_up_spawners.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/burning.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/flame_power_up.hpp>
#include <bim/game/component/flame_power_up_spawner.hpp>
#include <bim/game/component/position_on_grid.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(update_flame_power_up_spawners, burning)
{
  entt::registry registry;
  bim::game::arena arena(3, 3);

  const entt::entity entity = registry.create();
  registry.emplace<bim::game::flame_power_up_spawner>(entity);
  registry.emplace<bim::game::burning>(entity);
  registry.emplace<bim::game::position_on_grid>(entity, 1, 2);

  bim::game::update_flame_power_up_spawners(registry, arena);

  const entt::entity power_up = arena.entity_at(1, 2);
  EXPECT_TRUE(entt::null != power_up);
  EXPECT_TRUE(
      registry.storage<bim::game::flame_power_up>().contains(power_up));
}
