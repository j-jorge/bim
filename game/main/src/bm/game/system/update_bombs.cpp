/*
  Copyright (C) 2023 Julien Jorge

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <bm/game/arena.hpp>

#include <bm/game/system/update_bombs.hpp>

#include <bm/game/component/bomb.hpp>
#include <bm/game/component/burning.hpp>
#include <bm/game/component/flame_direction.hpp>
#include <bm/game/component/position_on_grid.hpp>

#include <entt/entity/registry.hpp>

static bool burn(entt::registry& registry, bm::game::arena& arena,
                 std::uint8_t x, std::uint8_t y,
                 bm::game::flame_horizontal horizontal,
                 bm::game::flame_vertical vertical, bm::game::flame_end end)
{
  if (arena.is_static_wall(x, y))
    return false;

  const entt::entity entity = arena.entity_at(x, y);

  if (entity != entt::null)
    {
      if (!registry.storage<bm::game::burning>().contains(entity))
        registry.emplace<bm::game::burning>(entity);

      return false;
    }
  else
    arena.put_entity(
        x, y,
        bm::game::flame_factory(registry, x, y, horizontal, vertical, end));

  return true;
}

static void create_flames(entt::registry& registry, bm::game::arena& arena,
                          bm::game::position_on_grid p, std::uint8_t strength)
{
  // On the left.
  for (std::uint8_t offset = 1; offset <= strength; ++offset)
    if ((p.x < offset)
        || !burn(registry, arena, p.x - offset, p.y,
                 bm::game::flame_horizontal::yes, bm::game::flame_vertical::no,
                 (offset == strength) ? bm::game::flame_end::yes
                                      : bm::game::flame_end::no))
      break;

  // On the right.
  for (std::uint8_t offset = 1, width = arena.width(); offset <= strength;
       ++offset)
    if ((p.x + offset == width)
        || !burn(registry, arena, p.x + offset, p.y,
                 bm::game::flame_horizontal::yes, bm::game::flame_vertical::no,
                 (offset == strength) ? bm::game::flame_end::yes
                                      : bm::game::flame_end::no))
      break;

  // Above.
  for (std::uint8_t offset = 1; offset <= strength; ++offset)
    if ((p.y < offset)
        || !burn(registry, arena, p.x, p.y - offset,
                 bm::game::flame_horizontal::no, bm::game::flame_vertical::yes,
                 (offset == strength) ? bm::game::flame_end::yes
                                      : bm::game::flame_end::no))
      break;

  // Below.
  for (std::uint8_t offset = 1, height = arena.height(); offset <= strength;
       ++offset)
    if ((p.y + offset == height)
        || !burn(registry, arena, p.x, p.y + offset,
                 bm::game::flame_horizontal::no, bm::game::flame_vertical::yes,
                 (offset == strength) ? bm::game::flame_end::yes
                                      : bm::game::flame_end::no))
      break;

  arena.put_entity(p.x, p.y,
                   bm::game::flame_factory(registry, p.x, p.y,
                                           bm::game::flame_horizontal::yes,
                                           bm::game::flame_vertical::yes,
                                           bm::game::flame_end::no));
}

void bm::game::update_bombs(entt::registry& registry, arena& arena,
                            std::chrono::milliseconds elapsed_time)
{
  registry.view<bomb, burning>().each(
      [&](bomb& b) -> void
      {
        b.duration_until_explosion = std::chrono::seconds(0);
      });

  registry.view<bomb, position_on_grid>().each(
      [&](entt::entity e, bomb& b, position_on_grid position) -> void
      {
        if (elapsed_time >= b.duration_until_explosion)
          {
            registry.destroy(e);
            arena.erase_entity(position.x, position.y);
            create_flames(registry, arena, position, b.strength);
          }
        else
          b.duration_until_explosion -= elapsed_time;
      });
}
