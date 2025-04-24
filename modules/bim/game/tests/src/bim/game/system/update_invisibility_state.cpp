// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_invisibility_state.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/invisibility_state.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/fill_context.hpp>
#include <bim/game/factory/invisibility_state.hpp>
#include <bim/game/factory/player.hpp>
#include <bim/game/system/remove_dead_objects.hpp>
#include <bim/game/system/update_timers.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(update_invisibility_state, expire)
{
  bim::game::context context;
  bim::game::fill_context(context);

  entt::registry registry;
  bim::game::arena arena(3, 3);
  const int x = 1;
  const int y = 1;

  const entt::entity player_entity =
      bim::game::player_factory(registry, 0, x, y, bim::game::animation_id{});

  const std::chrono::milliseconds duration(100);

  bim::game::invisibility_state_factory(registry, player_entity, duration);

  EXPECT_TRUE(bim::game::is_invisible(registry, player_entity));

  bim::game::update_timers(registry, duration);
  bim::game::update_invisibility_state(context, registry);
  bim::game::remove_dead_objects(registry);

  EXPECT_FALSE(bim::game::is_invisible(registry, player_entity));
}