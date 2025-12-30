// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_flames.hpp>

#include <bim/game/component/burning.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/entity_world_map.hpp>

#include <entt/entity/registry.hpp>

void bim::game::update_flames(entt::registry& registry,
                              entity_world_map& entity_map)
{
  // Burn any entity in contact with a flame.
  registry.view<flame, position_on_grid, timer>().each(
      [&](entt::entity e, const flame& f, const position_on_grid& position,
          const timer& t) -> void
      {
        if (t.duration.count() == 0)
          registry.emplace_or_replace<dead>(e);
        else
          for (entt::entity o : entity_map.entities_at(position.x, position.y))
            registry.emplace_or_replace<burning>(o);
      });
}
