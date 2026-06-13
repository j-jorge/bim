// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_clocks.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/clock.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(update_clocks, duration)
{
  entt::registry registry;

  const entt::entity entity = registry.create();
  const bim::game::clock& clock = registry.emplace<bim::game::clock>(
      entity, std::chrono::milliseconds(24));

  EXPECT_EQ(std::chrono::milliseconds(24), clock.date);

  bim::game::update_clocks(registry, std::chrono::milliseconds(12));
  EXPECT_EQ(std::chrono::milliseconds(36), clock.date);
}
