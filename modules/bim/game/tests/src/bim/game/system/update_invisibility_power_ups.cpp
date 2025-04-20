// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_invisibility_power_ups.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/dead.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/constant/invisibility_duration.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/fill_context.hpp>
#include <bim/game/factory/invisibility_power_up.hpp>
#include <bim/game/factory/player.hpp>
#include <bim/game/system/update_players.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(update_invisibility_power_ups, player_collision)
{
  bim::game::context context;
  bim::game::fill_context(context);

  entt::registry registry;
  bim::game::arena arena(3, 3);
  const int x = 1;
  const int y = 1;

  const entt::entity power_up_entity =
      invisibility_power_up_factory(registry, arena, x, y);
  const entt::entity player_entity =
      bim::game::player_factory(registry, 0, x, y, bim::game::animation_id{});

  const bim::game::player& player =
      registry.get<bim::game::player>(player_entity);

  bim::game::update_invisibility_power_ups(registry, arena);
  bim::game::update_players(context, registry, arena);

  EXPECT_TRUE(player.invisible);
  EXPECT_TRUE(registry.storage<bim::game::dead>().contains(power_up_entity));
}

TEST(update_invisibility_power_ups, two_players_only_one_get_the_power_up)
{
  bim::game::context context;
  bim::game::fill_context(context);

  entt::registry registry;
  bim::game::arena arena(3, 3);
  const int x = 1;
  const int y = 1;

  invisibility_power_up_factory(registry, arena, x, y);

  const entt::entity player_entity[2] = {
    bim::game::player_factory(registry, 0, x, y, bim::game::animation_id{}),
    bim::game::player_factory(registry, 1, x, y, bim::game::animation_id{})
  };

  bim::game::player* players[2] = {
    &registry.get<bim::game::player>(player_entity[0]),
    &registry.get<bim::game::player>(player_entity[1])
  };

  EXPECT_EQ(players[0]->invisible, players[1]->invisible);

  bim::game::update_invisibility_power_ups(registry, arena);
  bim::game::update_players(context, registry, arena);

  EXPECT_TRUE(players[0]->invisible ^ players[1]->invisible);
}

