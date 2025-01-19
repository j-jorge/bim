// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/factory/bomb.hpp>

#include <bim/game/component/bomb.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/timer.hpp>

#include <entt/entity/registry.hpp>

entt::entity bim::game::bomb_factory(entt::registry& registry, std::uint8_t x,
                                     std::uint8_t y, std::uint8_t strength,
                                     std::uint8_t player_index)
{
  return bomb_factory(registry, x, y, strength, player_index,
                      std::chrono::seconds(3));
}

entt::entity
bim::game::bomb_factory(entt::registry& registry, std::uint8_t x,
                        std::uint8_t y, std::uint8_t strength,
                        std::uint8_t player_index,
                        std::chrono::milliseconds duration_until_explosion)
{
  const entt::entity entity = registry.create();

  registry.emplace<bomb>(entity, strength, player_index);
  registry.emplace<position_on_grid>(entity, x, y);
  registry.emplace<timer>(entity, duration_until_explosion);

  return entity;
}
