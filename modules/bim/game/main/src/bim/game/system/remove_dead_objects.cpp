// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/remove_dead_objects.hpp>

#include <bim/game/component/crushed.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/invincibility_state.hpp>
#include <bim/game/component/invisibility_state.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/entity_world_map.hpp>

#include <entt/entity/registry.hpp>

void bim::game::remove_dead_objects(entt::registry& registry,
                                    entity_world_map& entity_map)
{
  // All crushed objects are dead.
  registry.view<crushed>().each(
      [&](entt::entity e) -> void
      {
        registry.emplace_or_replace<dead>(e);
      });

  // Kill entities attached to dead entities too.
  registry.view<dead, invincibility_state>().each(
      [&](const invincibility_state& s) -> void
      {
        registry.emplace_or_replace<dead>(s.entity);
      });

  registry.view<dead, invisibility_state>().each(
      [&](const invisibility_state& s) -> void
      {
        registry.emplace_or_replace<dead>(s.entity);
      });

  // Remove dead entities from the entity map.
  registry.view<dead, fractional_position_on_grid>().each(
      [&](entt::entity e, const fractional_position_on_grid& p) -> void
      {
        entity_map.erase_entity(e, p.grid_aligned_x(), p.grid_aligned_y());
      });

  registry.view<dead, position_on_grid>().each(
      [&](entt::entity e, const position_on_grid& p) -> void
      {
        entity_map.erase_entity(e, p.x, p.y);
      });

  // Finally, remove the dead entities from the registry.
  registry.view<dead>().each(
      [&](entt::entity e) -> void
      {
        registry.destroy(e);
      });
}
