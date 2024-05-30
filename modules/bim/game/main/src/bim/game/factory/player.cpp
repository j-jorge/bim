#include <bim/game/factory/player.hpp>

#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action_queue.hpp>
#include <bim/game/component/player_direction.hpp>

#include <entt/entity/registry.hpp>

entt::entity bim::game::player_factory(entt::registry& registry,
                                       std::uint8_t index, std::uint8_t cell_x,
                                       std::uint8_t cell_y)
{
  const entt::entity entity = registry.create();

  registry.emplace<player>(entity, index, player_direction::down, 1, 1, 1);

  constexpr fractional_position_on_grid::value_type half =
      fractional_position_on_grid::value_type(1) / 2;
  registry.emplace<fractional_position_on_grid>(entity, cell_x + half,
                                                cell_y + half);

  registry.emplace<player_action>(entity);
  registry.emplace<player_action_queue>(entity);

  return entity;
}
