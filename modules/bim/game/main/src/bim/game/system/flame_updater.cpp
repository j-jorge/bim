// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/flame_updater.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/burning.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/flame_blocker.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/timer.hpp>

#include <entt/entity/registry.hpp>

bim::game::flame_updater::flame_updater(std::uint8_t arena_width,
                                        std::uint8_t arena_height)
  : m_flame_map(arena_width, arena_height)
{}

bim::game::flame_updater::~flame_updater() = default;

void bim::game::flame_updater::update(entt::registry& registry, arena& arena)
{
  m_flame_map.fill(false);

  // Collect the active flames.
  registry.view<flame, position_on_grid, timer>().each(
      [&](entt::entity e, const flame& f, const position_on_grid& position,
          const timer& t) -> void
      {
        if (t.duration.count() == 0)
          registry.emplace_or_replace<dead>(e);
        else
          m_flame_map(position.x, position.y) = true;
      });

  // Burn any moving object in contact with a flame.
  registry.view<fractional_position_on_grid>().each(
      [&](entt::entity e, const fractional_position_on_grid& position) -> void
      {
        if (m_flame_map(position.grid_aligned_x(), position.grid_aligned_y()))
          registry.emplace_or_replace<bim::game::burning>(e);
      });

  // Remove the flame blockers we don't need.
  registry.view<flame_blocker, position_on_grid, timer>().each(
      [&](entt::entity e, const position_on_grid& position,
          const timer& t) -> void
      {
        if (t.duration.count() == 0)
          {
            arena.erase_entity(position.x, position.y);
            registry.emplace_or_replace<dead>(e);
          }
      });
}
