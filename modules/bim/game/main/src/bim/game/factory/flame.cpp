// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/factory/flame.hpp>

#include <bim/game/component/flame.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/constant/flame_duration.hpp>

#include <entt/entity/registry.hpp>

entt::entity bim::game::flame_factory(entt::registry& registry, std::uint8_t x,
                                      std::uint8_t y,
                                      flame_direction direction,
                                      flame_segment segment)
{
  return flame_factory(registry, x, y, direction, segment, g_flame_duration);
}

entt::entity bim::game::flame_factory(entt::registry& registry, std::uint8_t x,
                                      std::uint8_t y,
                                      flame_direction direction,
                                      flame_segment segment,
                                      std::chrono::milliseconds time_to_live)
{
  const entt::entity entity = registry.create();

  registry.emplace<flame>(entity, direction, segment, time_to_live);
  registry.emplace<position_on_grid>(entity, x, y);
  registry.emplace<timer>(entity, time_to_live);

  return entity;
}
