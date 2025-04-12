// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_bombs.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/bomb.hpp>
#include <bim/game/component/burning.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/flame_direction.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/factory/flame.hpp>

#include <entt/entity/registry.hpp>

static bool burn(entt::registry& registry, bim::game::arena& arena,
                 std::uint8_t x, std::uint8_t y,
                 bim::game::flame_direction direction,
                 bim::game::flame_segment segment)
{
  if (arena.is_static_wall(x, y))
    return false;

  const entt::entity entity = arena.entity_at(x, y);

  if (entity != entt::null)
    {
      if (!registry.storage<bim::game::burning>().contains(entity))
        registry.emplace<bim::game::burning>(entity);

      return false;
    }
  else
    arena.put_entity(
        x, y, bim::game::flame_factory(registry, x, y, direction, segment));

  return true;
}

static void create_flames(entt::registry& registry, bim::game::arena& arena,
                          bim::game::position_on_grid p, std::uint8_t strength)
{
  // On the left.
  for (std::uint8_t offset = 1; offset <= strength; ++offset)
    if ((p.x < offset)
        || !burn(registry, arena, p.x - offset, p.y,
                 bim::game::flame_direction::left,
                 (offset == strength) ? bim::game::flame_segment::tip
                                      : bim::game::flame_segment::arm))
      break;

  // On the right.
  for (std::uint8_t offset = 1, width = arena.width(); offset <= strength;
       ++offset)
    if ((p.x + offset == width)
        || !burn(registry, arena, p.x + offset, p.y,
                 bim::game::flame_direction::right,
                 (offset == strength) ? bim::game::flame_segment::tip
                                      : bim::game::flame_segment::arm))
      break;

  // Above.
  for (std::uint8_t offset = 1; offset <= strength; ++offset)
    if ((p.y < offset)
        || !burn(registry, arena, p.x, p.y - offset,
                 bim::game::flame_direction::up,
                 (offset == strength) ? bim::game::flame_segment::tip
                                      : bim::game::flame_segment::arm))
      break;

  // Below.
  for (std::uint8_t offset = 1, height = arena.height(); offset <= strength;
       ++offset)
    if ((p.y + offset == height)
        || !burn(registry, arena, p.x, p.y + offset,
                 bim::game::flame_direction::down,
                 (offset == strength) ? bim::game::flame_segment::tip
                                      : bim::game::flame_segment::arm))
      break;

  // Starting point, the direction does not matter.
  arena.put_entity(p.x, p.y,
                   bim::game::flame_factory(registry, p.x, p.y,
                                            bim::game::flame_direction::up,
                                            bim::game::flame_segment::origin));
}

void bim::game::update_bombs(entt::registry& registry, arena& arena)
{
  registry.view<bomb, timer, burning>().each(
      [&](const bomb&, timer& t) -> void
      {
        t.duration = std::chrono::seconds(0);
      });

  registry.view<bomb, position_on_grid, timer>().each(
      [&](entt::entity e, const bomb& b, const position_on_grid& position,
          const timer& t) -> void
      {
        if (t.duration.count() > 0)
          return;

        arena.erase_entity(position.x, position.y);
        create_flames(registry, arena, position, b.strength);
        registry.emplace_or_replace<dead>(e);
      });
}
