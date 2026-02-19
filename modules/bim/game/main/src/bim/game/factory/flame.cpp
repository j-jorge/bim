// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/factory/flame.hpp>

#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/flame_animations.hpp>

#include <entt/entity/registry.hpp>

entt::entity bim::game::flame_factory(const context& context,
                                      entt::registry& registry, std::uint8_t x,
                                      std::uint8_t y,
                                      flame_direction direction,
                                      flame_segment segment)
{
  const entt::entity entity = registry.create();

  const bim::game::animation_id initial_state =
      context.get<const flame_animations>().warm_up;

  registry.emplace<animation_state>(entity, initial_state,
                                    std::chrono::seconds());
  registry.emplace<flame>(entity, direction, segment);
  registry.emplace<position_on_grid>(entity, x, y);

  return entity;
}
