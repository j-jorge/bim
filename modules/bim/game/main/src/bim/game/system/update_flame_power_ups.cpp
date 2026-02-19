// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_flame_power_ups.hpp>

#include <bim/game/component/dead.hpp>
#include <bim/game/component/flame_power_up.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/constant/max_bomb_strength.hpp>
#include <bim/game/entity_world_map.hpp>

#include <entt/entity/registry.hpp>

static void check_flame_power_up_player_collision(
    entt::registry& registry, bim::game::entity_world_map& entity_map,
    bim::game::player& p, bim::game::fractional_position_on_grid position)
{
  const std::uint8_t x = position.grid_aligned_x();
  const std::uint8_t y = position.grid_aligned_y();

  for (entt::entity maybe_flame_power_up : entity_map.entities_at(x, y))
    {
      if (registry.storage<bim::game::dead>().contains(maybe_flame_power_up))
        continue;

      if (registry.storage<bim::game::flame_power_up>().contains(
              maybe_flame_power_up))
        {
          p.bomb_strength =
              std::min(p.bomb_strength + 1, bim::game::g_max_bomb_strength);

          entity_map.erase_entity(maybe_flame_power_up, x, y);
          registry.emplace<bim::game::dead>(maybe_flame_power_up);
          break;
        }
    }
}

void bim::game::update_flame_power_ups(entt::registry& registry,
                                       entity_world_map& entity_map)
{
  registry.view<player, fractional_position_on_grid>().each(
      [&](entt::entity, player& p,
          fractional_position_on_grid position) -> void
      {
        check_flame_power_up_player_collision(registry, entity_map, p,
                                              position);
      });
}
