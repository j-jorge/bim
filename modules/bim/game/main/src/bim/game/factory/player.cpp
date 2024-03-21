#include <bim/game/factory/player.hpp>

#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action.hpp>
#include <bim/game/component/position_on_grid.hpp>

#include <entt/entity/registry.hpp>

entt::entity bim::game::player_factory(entt::registry& registry,
                                       std::uint8_t index, std::uint8_t x,
                                       std::uint8_t y)
{
  const entt::entity entity = registry.create();

  registry.emplace<player>(entity, index, player_direction::down, 1);
  registry.emplace<position_on_grid>(entity, position_on_grid{ x, y });
  registry.emplace<player_action>(entity);

  return entity;
}
