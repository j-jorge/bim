// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_invisibility_power_ups.hpp>

#include <bim/game/component/dead.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/invisibility_power_up.hpp>
#include <bim/game/component/invisibility_state.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/entity_world_map.hpp>
#include <bim/game/factory/invisibility_state.hpp>

#include <entt/entity/registry.hpp>

static void check_invisibility_power_up_player_collision(
    entt::registry& registry, bim::game::entity_world_map& entity_map,
    entt::entity e, bim::game::fractional_position_on_grid position)
{
  const std::uint8_t x = position.grid_aligned_x();
  const std::uint8_t y = position.grid_aligned_y();

  for (entt::entity maybe_invisibility_power_up : entity_map.entities_at(x, y))
    {
      if (registry.storage<bim::game::dead>().contains(
              maybe_invisibility_power_up))
        continue;

      if (registry.storage<bim::game::invisibility_power_up>().contains(
              maybe_invisibility_power_up))
        {
          bim::game::invisibility_state_factory(registry, e,
                                                std::chrono::seconds(15));

          entity_map.erase_entity(maybe_invisibility_power_up, x, y);
          registry.emplace<bim::game::dead>(maybe_invisibility_power_up);
          break;
        }
    }
}

void bim::game::update_invisibility_power_ups(
    entt::registry& registry, bim::game::entity_world_map& entity_map)
{
  registry.view<player, fractional_position_on_grid>().each(
      [&](entt::entity e, player&,
          fractional_position_on_grid position) -> void
      {
        check_invisibility_power_up_player_collision(registry, entity_map, e,
                                                     position);
      });
}
