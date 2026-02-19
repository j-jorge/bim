// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/factory/crate.hpp>

#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/crate.hpp>
#include <bim/game/component/layer_zero.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/solid.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/crate_animations.hpp>
#include <bim/game/entity_world_map.hpp>

#include <entt/entity/registry.hpp>

entt::entity bim::game::crate_factory(const context& context,
                                      entt::registry& registry,
                                      entity_world_map& entity_map,
                                      std::uint8_t x, std::uint8_t y)
{
  const entt::entity entity = registry.create();

  const bim::game::animation_id initial_state =
      context.get<const crate_animations>().idle;

  registry.emplace<animation_state>(entity, initial_state,
                                    std::chrono::seconds());
  registry.emplace<crate>(entity);
  registry.emplace<layer_zero>(entity);
  registry.emplace<position_on_grid>(entity, x, y);
  registry.emplace<solid>(entity);

  entity_map.put_entity(entity, x, y);

  return entity;
}
