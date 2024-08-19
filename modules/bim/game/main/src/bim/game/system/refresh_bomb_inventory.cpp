// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/refresh_bomb_inventory.hpp>

#include <bim/game/component/bomb.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/constant/max_player_count.hpp>

#include <entt/entity/registry.hpp>

void bim::game::refresh_bomb_inventory(entt::registry& registry)
{
  std::uint8_t active_bombs_per_player[g_max_player_count] = { 0 };

  registry.view<bomb>().each(
      [&](const bomb& bomb) -> void
      {
        assert(bomb.player_index < g_max_player_count);
        ++active_bombs_per_player[bomb.player_index];
      });

  registry.view<player>().each(
      [&](player& player) -> void
      {
        player.bomb_available =
            player.bomb_capacity - active_bombs_per_player[player.index];
      });
}
