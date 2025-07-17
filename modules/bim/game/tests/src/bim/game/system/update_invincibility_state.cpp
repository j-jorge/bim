// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_invincibility_state.hpp>

#include <bim/game/component/invincibility_state.hpp>
#include <bim/game/factory/invincibility_state.hpp>
#include <bim/game/system/remove_dead_objects.hpp>
#include <bim/game/system/update_timers.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(update_invincibility_state, expire)
{
  entt::registry registry;

  const entt::entity e = registry.create();
  const std::chrono::milliseconds duration(100);

  bim::game::invincibility_state_factory(registry, e, duration);

  EXPECT_TRUE(registry.storage<bim::game::invincibility_state>().contains(e));

  bim::game::update_timers(registry, duration);
  bim::game::update_invincibility_state(registry);
  bim::game::remove_dead_objects(registry);

  EXPECT_FALSE(registry.storage<bim::game::invincibility_state>().contains(e));
}
