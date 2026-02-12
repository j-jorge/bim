// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_flames.hpp>

#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/burning.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/flame_animations.hpp>
#include <bim/game/entity_world_map.hpp>

#include <entt/entity/registry.hpp>

void bim::game::update_flames(const context& context, entt::registry& registry,
                              entity_world_map& entity_map)
{
  const flame_animations& animations = context.get<const flame_animations>();

  // Burn any entity in contact with a flame.
  registry.view<flame, position_on_grid, animation_state>().each(
      [&](const flame& f, const position_on_grid& position,
          const animation_state& state) -> void
      {
        if (animations.is_burning(state.model))
          for (entt::entity e : entity_map.entities_at(position.x, position.y))
            registry.emplace_or_replace<burning>(e);
      });
}
