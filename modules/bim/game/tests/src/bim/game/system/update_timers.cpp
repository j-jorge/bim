// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_timers.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/timer.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(update_timers, duration)
{
  entt::registry registry;

  const entt::entity entity = registry.create();
  bim::game::timer& timer = registry.emplace<bim::game::timer>(
      entity, std::chrono::milliseconds(24));

  EXPECT_EQ(std::chrono::milliseconds(24), timer.duration);

  bim::game::update_timers(registry, std::chrono::milliseconds(12));
  EXPECT_EQ(std::chrono::milliseconds(12), timer.duration);
}
