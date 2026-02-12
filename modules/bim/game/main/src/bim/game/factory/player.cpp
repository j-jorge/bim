// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/factory/player.hpp>

#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action_queue.hpp>
#include <bim/game/component/shallow.hpp>
#include <bim/game/entity_world_map.hpp>

#include <entt/entity/registry.hpp>

entt::entity bim::game::player_factory(entt::registry& registry,
                                       entity_world_map& entity_map,
                                       std::uint8_t index, std::uint8_t cell_x,
                                       std::uint8_t cell_y,
                                       animation_id initial_state)
{
  const entt::entity entity = registry.create();

  registry.emplace<player>(entity, index, 1, 1, 1);

  constexpr fractional_position_on_grid::value_type half =
      fractional_position_on_grid::value_type(1) / 2;
  registry.emplace<fractional_position_on_grid>(entity, cell_x + half,
                                                cell_y + half);

  registry.emplace<player_action>(entity);
  registry.emplace<player_action_queue>(entity);
  registry.emplace<animation_state>(entity, initial_state,
                                    std::chrono::seconds());
  registry.emplace<shallow>(entity);

  entity_map.put_entity(entity, cell_x, cell_y);

  return entity;
}
