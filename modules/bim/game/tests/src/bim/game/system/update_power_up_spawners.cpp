// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_power_up_spawners.hpp>

#include <bim/game/entity_world_map.hpp>

#include <bim/game/component/bomb_power_up.hpp>
#include <bim/game/component/bomb_power_up_spawner.hpp>
#include <bim/game/component/burning.hpp>
#include <bim/game/component/flame_power_up.hpp>
#include <bim/game/component/flame_power_up_spawner.hpp>
#include <bim/game/component/invisibility_power_up.hpp>
#include <bim/game/component/invisibility_power_up_spawner.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/shield_power_up.hpp>
#include <bim/game/component/shield_power_up_spawner.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

namespace
{
  template <typename Spawner, typename PowerUp>
  struct spawner_and_power_up
  {
    using spawner = Spawner;
    using power_up = PowerUp;
  };
}

template <typename T>
class update_power_up_spawners_test : public testing::Test
{};

using spawner_and_power_ups = testing::Types<
    spawner_and_power_up<bim::game::bomb_power_up_spawner,
                         bim::game::bomb_power_up>,
    spawner_and_power_up<bim::game::flame_power_up_spawner,
                         bim::game::flame_power_up>,
    spawner_and_power_up<bim::game::invisibility_power_up_spawner,
                         bim::game::invisibility_power_up>,
    spawner_and_power_up<bim::game::shield_power_up_spawner,
                         bim::game::shield_power_up>>;

TYPED_TEST_SUITE(update_power_up_spawners_test, spawner_and_power_ups);

TYPED_TEST(update_power_up_spawners_test, burning)
{
  using spawner_type = TypeParam::spawner;
  using power_up_type = TypeParam::power_up;

  entt::registry registry;
  bim::game::entity_world_map entity_map(3, 3);

  const entt::entity entity = registry.create();
  registry.emplace<spawner_type>(entity);
  registry.emplace<bim::game::burning>(entity);
  registry.emplace<bim::game::position_on_grid>(entity, 1, 2);

  bim::game::update_power_up_spawners<spawner_type>(registry, entity_map);

  const std::span<const entt::entity> power_ups = entity_map.entities_at(1, 2);
  ASSERT_EQ(1, power_ups.size());
  EXPECT_TRUE(registry.storage<power_up_type>().contains(power_ups[0]));
}
