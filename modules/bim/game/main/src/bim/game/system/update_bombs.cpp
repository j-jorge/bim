// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_bombs.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/entity_world_map.hpp>

#include <bim/game/component/bomb.hpp>
#include <bim/game/component/burning.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/flame_blocker.hpp>
#include <bim/game/component/flame_direction.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/shallow.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/constant/flame_duration.hpp>
#include <bim/game/factory/flame.hpp>

#include <entt/entity/registry.hpp>

#include <boost/container/static_vector.hpp>

static bool burn(entt::registry& registry, const bim::game::arena& arena,
                 bim::game::entity_world_map& entity_map, std::uint8_t x,
                 std::uint8_t y, bim::game::flame_direction direction,
                 bim::game::flame_segment segment)
{
  if (arena.is_static_wall(x, y))
    return false;

  const std::span<const entt::entity> entities = entity_map.entities_at(x, y);
  bool flames_go_through = true;

  for (const entt::entity entity : entities)
    {
      flames_go_through &=
          registry.storage<bim::game::shallow>().contains(entity);
      registry.emplace_or_replace<bim::game::burning>(entity);
    }

  if (flames_go_through)
    bim::game::flame_factory(registry, x, y, direction, segment);

  return flames_go_through;
}

static void create_flames(entt::registry& registry,
                          const bim::game::arena& arena,
                          bim::game::entity_world_map& entity_map,
                          bim::game::position_on_grid p, std::uint8_t strength)
{
  // On the left.
  for (std::uint8_t offset = 1; offset <= strength; ++offset)
    if ((p.x < offset)
        || !burn(registry, arena, entity_map, p.x - offset, p.y,
                 bim::game::flame_direction::left,
                 (offset == strength) ? bim::game::flame_segment::tip
                                      : bim::game::flame_segment::arm))
      break;

  // On the right.
  for (std::uint8_t offset = 1, width = arena.width(); offset <= strength;
       ++offset)
    if ((p.x + offset == width)
        || !burn(registry, arena, entity_map, p.x + offset, p.y,
                 bim::game::flame_direction::right,
                 (offset == strength) ? bim::game::flame_segment::tip
                                      : bim::game::flame_segment::arm))
      break;

  // Above.
  for (std::uint8_t offset = 1; offset <= strength; ++offset)
    if ((p.y < offset)
        || !burn(registry, arena, entity_map, p.x, p.y - offset,
                 bim::game::flame_direction::up,
                 (offset == strength) ? bim::game::flame_segment::tip
                                      : bim::game::flame_segment::arm))
      break;

  // Below.
  for (std::uint8_t offset = 1, height = arena.height(); offset <= strength;
       ++offset)
    if ((p.y + offset == height)
        || !burn(registry, arena, entity_map, p.x, p.y + offset,
                 bim::game::flame_direction::down,
                 (offset == strength) ? bim::game::flame_segment::tip
                                      : bim::game::flame_segment::arm))
      break;

  // Starting point, the direction does not matter.
  bim::game::flame_factory(registry, p.x, p.y, bim::game::flame_direction::up,
                           bim::game::flame_segment::origin);

  // Put something where the bomb was, in order to stop the propagation of the
  // future flames until the current one is off.
  const entt::entity e = registry.create();
  registry.emplace<bim::game::position_on_grid>(e, p);
  registry.emplace<bim::game::timer>(e, bim::game::g_flame_duration);
  registry.emplace<bim::game::flame_blocker>(e);

  entity_map.put_entity(e, p.x, p.y);
}

void bim::game::update_bombs(entt::registry& registry, arena& arena,
                             bim::game::entity_world_map& entity_map)
{
  registry.view<bomb, timer, burning>().each(
      [&](const bomb&, timer& t) -> void
      {
        t.duration = std::chrono::seconds(0);
      });

  // Remove the flame blockers we don't need.
  registry.view<flame_blocker, position_on_grid, timer>().each(
      [&](entt::entity e, const position_on_grid& position,
          const timer& t) -> void
      {
        if (t.duration.count() == 0)
          {
            entity_map.erase_entity(e, position.x, position.y);
            registry.emplace_or_replace<dead>(e);
          }
      });

  registry.view<bomb, position_on_grid, timer>().each(
      [&](entt::entity e, const bomb& b, const position_on_grid& position,
          const timer& t) -> void
      {
        if (t.duration.count() > 0)
          return;

        create_flames(registry, arena, entity_map, position, b.strength);
        entity_map.erase_entity(e, position.x, position.y);
        registry.emplace_or_replace<dead>(e);
      });
}
