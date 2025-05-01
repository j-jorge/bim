// SPDX-License-Identifier: AGPL-3.0-only
// bim/modules/bim/game/tests/src/bim/game/factory/power_up.cpp
#include <bim/game/arena.hpp>

#include "bim/game/component/position_on_grid.hpp"
#include <bim/game/component/bomb_power_up.hpp>
#include <bim/game/component/flame_power_up.hpp>
#include <bim/game/component/invisibility_power_up.hpp>

#include <bim/game/factory/power_up.hpp>

#include <entt/entity/registry.hpp>
#include <entt/entt.hpp>

#include <gtest/gtest.h>

template <typename T> class PowerUpFactoryTest : public testing::Test {};

using PowerUpTypes =
    testing::Types<bim::game::bomb_power_up, bim::game::flame_power_up,
                   bim::game::invisibility_power_up>;

TYPED_TEST_SUITE(PowerUpFactoryTest, PowerUpTypes);

TYPED_TEST(PowerUpFactoryTest, CreatesAtCorrectPosition) {
  constexpr std::uint8_t x = 1;
  constexpr std::uint8_t y = 0;

  bim::game::arena arena(2, 2);

  entt::registry registry;

  auto entity = bim::game::power_up_factory<TypeParam>(registry, arena, x, y);

  // Verifications
  ASSERT_NE(entity, static_cast<entt::entity>(entt::null));
  ASSERT_TRUE(registry.valid(entity));

  auto &pos = registry.get<bim::game::position_on_grid>(entity);

  ASSERT_EQ(pos.x, x);
  ASSERT_EQ(pos.y, y);

  ASSERT_TRUE(registry.all_of<TypeParam>(entity));
}
