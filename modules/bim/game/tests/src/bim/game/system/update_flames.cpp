// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_flames.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/flame.hpp>
#include <bim/game/component/flame_direction.hpp>
#include <bim/game/factory/flame.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(update_flames, time_to_live)
{
  entt::registry registry;
  bim::game::arena arena(3, 3);

  const entt::entity entity = bim::game::flame_factory(
      registry, 0, 0, bim::game::flame_direction::up,
      bim::game::flame_segment::origin, std::chrono::milliseconds(24));
  bim::game::flame& flame = registry.get<bim::game::flame>(entity);

  EXPECT_EQ(std::chrono::milliseconds(24), flame.time_to_live);

  bim::game::update_flames(registry, arena, std::chrono::milliseconds(12));
  EXPECT_EQ(std::chrono::milliseconds(12), flame.time_to_live);
}
