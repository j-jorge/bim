// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_shield_power_ups.hpp>

#include <bim/game/component/burning.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/shield.hpp>
#include <bim/game/component/shield_power_up.hpp>
#include <bim/game/constant/max_bomb_strength.hpp>
#include <bim/game/entity_world_map.hpp>

#include <entt/entity/registry.hpp>

static void check_shield_power_up_player_collision(
    entt::registry& registry, bim::game::entity_world_map& entity_map,
    entt::entity player_entity,
    bim::game::fractional_position_on_grid position)
{
  const std::uint8_t x = position.grid_aligned_x();
  const std::uint8_t y = position.grid_aligned_y();

  for (entt::entity maybe_shield_power_up : entity_map.entities_at(x, y))
    {
      if (registry.storage<bim::game::dead>().contains(maybe_shield_power_up))
        continue;

      if (registry.storage<bim::game::shield_power_up>().contains(
              maybe_shield_power_up))
        {
          registry.emplace_or_replace<bim::game::shield>(player_entity);
          entity_map.erase_entity(maybe_shield_power_up, x, y);
          registry.emplace<bim::game::dead>(maybe_shield_power_up);
          break;
        }
    }
}

void bim::game::update_shield_power_ups(entt::registry& registry,
                                        entity_world_map& entity_map)
{
  registry.view<shield_power_up, burning, position_on_grid>().each(
      [&](entt::entity e, position_on_grid position) -> void
      {
        entity_map.erase_entity(e, position.x, position.y);
        registry.emplace<dead>(e);
      });

  registry.view<player, fractional_position_on_grid>().each(
      [&](entt::entity e, const player&,
          fractional_position_on_grid position) -> void
      {
        check_shield_power_up_player_collision(registry, entity_map, e,
                                               position);
      });
}
