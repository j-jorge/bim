// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_invisibility_power_ups.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/burning.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/invisibility_power_up.hpp>
#include <bim/game/component/invisibility_state.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/factory/invisibility_state.hpp>

#include <entt/entity/registry.hpp>

static void check_invisibility_power_up_player_collision(
    entt::registry& registry, bim::game::arena& arena, entt::entity e,
    bim::game::fractional_position_on_grid position)
{
  const std::uint8_t x = position.grid_aligned_x();
  const std::uint8_t y = position.grid_aligned_y();

  const entt::entity colliding_entity = arena.entity_at(x, y);

  if (colliding_entity == entt::null)
    return;

  if (registry.storage<bim::game::dead>().contains(colliding_entity))
    return;

  if (registry.storage<bim::game::invisibility_power_up>().contains(
          colliding_entity))
    {
      bim::game::invisibility_state_factory(registry, e,
                                            std::chrono::milliseconds(7000));

      arena.erase_entity(x, y);
      registry.emplace<bim::game::dead>(colliding_entity);
    }
}

void bim::game::update_invisibility_power_ups(entt::registry& registry,
                                              arena& arena)
{
  registry.view<invisibility_power_up, burning, position_on_grid>().each(
      [&](entt::entity e, position_on_grid position) -> void
      {
        registry.emplace<dead>(e);
        arena.erase_entity(position.x, position.y);
      });

  registry.view<player, fractional_position_on_grid>().each(
      [&](entt::entity e, player&,
          fractional_position_on_grid position) -> void
      {
        check_invisibility_power_up_player_collision(registry, arena, e,
                                                     position);
      });
}
