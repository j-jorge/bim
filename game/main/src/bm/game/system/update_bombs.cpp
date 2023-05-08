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
#include <bm/game/component/flame.hpp>
#include <bm/game/component/position_on_grid.hpp>

#include <entt/entity/registry.hpp>

static bool burn(entt::registry& registry, bm::game::arena& arena, uint8_t x,
                 uint8_t y, bm::game::flame_horizontal horizontal,
                 bm::game::flame_vertical vertical, bm::game::flame_end end)
{
  if (arena.is_static_wall(x, y))
    return false;

  const entt::entity entity = arena.entity_at(x, y);

  if (entity != entt::null)
    {
      // burning: wall destroy, player death, bonus pop, flame horizontal
    }
  else
    // flame_factory(registry, x, y, horizontal, vertical, end);
    {

      const entt::entity flame_entity = registry.create();
      arena.put_entity(x, y, flame_entity);
      registry.emplace<bm::game::flame>(flame_entity, horizontal, vertical,
                                        end, std::chrono::milliseconds(800));

      registry.emplace<bm::game::position_on_grid>(flame_entity, x, y);
    }

  return true;
}

static void create_flames(entt::registry& registry, bm::game::arena& arena,
                          bm::game::position_on_grid p, uint8_t strength)
{
  // On the left.
  for (uint8_t offset = 1; offset <= strength; ++offset)
    if ((p.x < offset)
        || !burn(registry, arena, p.x - offset, p.y,
                 bm::game::flame_horizontal::yes, bm::game::flame_vertical::no,
                 (offset == strength) ? bm::game::flame_end::yes
                                      : bm::game::flame_end::no))
      break;

  // On the right.
  for (uint8_t offset = 1, width = arena.width(); offset <= strength; ++offset)
    if ((p.x + offset == width)
        || !burn(registry, arena, p.x + offset, p.y,
                 bm::game::flame_horizontal::yes, bm::game::flame_vertical::no,
                 (offset == strength) ? bm::game::flame_end::yes
                                      : bm::game::flame_end::no))
      break;

  // Above.
  for (uint8_t offset = 1; offset <= strength; ++offset)
    if ((p.y < offset)
        || !burn(registry, arena, p.x, p.y - offset,
                 bm::game::flame_horizontal::no, bm::game::flame_vertical::yes,
                 (offset == strength) ? bm::game::flame_end::yes
                                      : bm::game::flame_end::no))
      break;

  // Below.
  for (uint8_t offset = 1, height = arena.height(); offset <= strength;
       ++offset)
    if ((p.y + offset == height)
        || !burn(registry, arena, p.x, p.y + offset,
                 bm::game::flame_horizontal::no, bm::game::flame_vertical::yes,
                 (offset == strength) ? bm::game::flame_end::yes
                                      : bm::game::flame_end::no))
      break;

  // flame_factory(registry, p.x, p.y, bm::game::flame_horizontal::yes,
  // bm::game::flame_vertical::yes, bm::game::flame_end::no);
  const entt::entity flame_entity = registry.create();
  arena.put_entity(p.x, p.y, flame_entity);

  registry.emplace<bm::game::flame>(
      flame_entity, bm::game::flame_horizontal::yes,
      bm::game::flame_vertical::yes, bm::game::flame_end::no,
      std::chrono::milliseconds(800));

  registry.emplace<bm::game::position_on_grid>(flame_entity, p.x, p.y);
}

void bm::game::update_bombs(entt::registry& registry, arena& arena,
                            std::chrono::milliseconds elapsed_time)
{
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
