// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/factory/smoke_vfx.hpp>

#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/layer_front.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/smoke_animations.hpp>
#include <bim/game/entity_world_map.hpp>

#include <entt/entity/registry.hpp>

entt::entity bim::game::in_out_smoke_vfx_factory(
    const context& context, entt::registry& registry, std::uint8_t x,
    std::uint8_t y, std::chrono::milliseconds initial_time)
{
  const entt::entity entity = registry.create();

  const bim::game::animation_id initial_state =
      context.get<const smoke_animations>().in_out;

  registry.emplace<animation_state>(entity, initial_state, initial_time);
  registry.emplace<position_on_grid>(entity, x, y);
  registry.emplace<layer_front>(entity);

  return entity;
}
