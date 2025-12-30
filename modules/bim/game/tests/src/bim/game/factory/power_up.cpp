// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/arena.hpp>

#include <bim/game/component/bomb_power_up.hpp>
#include <bim/game/component/flame_power_up.hpp>
#include <bim/game/component/invisibility_power_up.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/shield_power_up.hpp>
#include <bim/game/entity_world_map.hpp>

#include <bim/game/factory/power_up.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

template <typename T>
class power_up_factory_test : public testing::Test
{};

using power_up_types =
    testing::Types<bim::game::bomb_power_up, bim::game::flame_power_up,
                   bim::game::invisibility_power_up,
                   bim::game::shield_power_up>;

TYPED_TEST_SUITE(power_up_factory_test, power_up_types);

TYPED_TEST(power_up_factory_test, creates_at_correct_position)
{
  constexpr std::uint8_t x = 1;
  constexpr std::uint8_t y = 0;

  bim::game::entity_world_map entity_map(2, 2);
  entt::registry registry;

  const entt::entity entity =
      bim::game::power_up_factory<TypeParam>(registry, entity_map, x, y);

  EXPECT_NE(entity, static_cast<entt::entity>(entt::null));
  EXPECT_TRUE(registry.valid(entity));

  const bim::game::position_on_grid& pos =
      registry.get<bim::game::position_on_grid>(entity);

  EXPECT_EQ(pos.x, x);
  EXPECT_EQ(pos.y, y);

  const std::span<const entt::entity> s = entity_map.entities_at(x, y);
  ASSERT_EQ(1, s.size());
  EXPECT_EQ(entity, s[0]);

  EXPECT_TRUE(registry.all_of<TypeParam>(entity));
}
