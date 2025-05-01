// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_invisibility_power_ups.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/dead.hpp>
#include <bim/game/component/invisibility_state.hpp>
#include <bim/game/factory/player.hpp>
#include <bim/game/factory/power_up.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

namespace bim::game
{
  struct invisibility_power_up;
}

TEST(update_invisibility_power_ups, player_collision)
{
  entt::registry registry;
  bim::game::arena arena(3, 3);
  const int x = 1;
  const int y = 1;

  const entt::entity power_up_entity =
      power_up_factory<bim::game::invisibility_power_up>(registry, arena, x,
                                                         y);
  const entt::entity player_entity =
      bim::game::player_factory(registry, 0, x, y, bim::game::animation_id{});

  bim::game::update_invisibility_power_ups(registry, arena);

  EXPECT_TRUE(bim::game::is_invisible(registry, player_entity));
  EXPECT_TRUE(registry.storage<bim::game::dead>().contains(power_up_entity));
}

TEST(update_invisibility_power_ups, two_players_only_one_gets_the_power_up)
{
  entt::registry registry;
  bim::game::arena arena(3, 3);
  const int x = 1;
  const int y = 1;

  power_up_factory<bim::game::invisibility_power_up>(registry, arena, x, y);

  const entt::entity player_entity[2] = {
    bim::game::player_factory(registry, 0, x, y, bim::game::animation_id{}),
    bim::game::player_factory(registry, 1, x, y, bim::game::animation_id{})
  };

  EXPECT_EQ(bim::game::is_invisible(registry, player_entity[0]),
            bim::game::is_invisible(registry, player_entity[1]));

  bim::game::update_invisibility_power_ups(registry, arena);

  EXPECT_NE(bim::game::is_invisible(registry, player_entity[0]),
            bim::game::is_invisible(registry, player_entity[1]));
}
