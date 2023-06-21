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
#include <bim/game/system/update_bombs.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/bomb.hpp>
#include <bim/game/component/burning.hpp>
#include <bim/game/component/flame_direction.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/factory/flame.hpp>

#include <entt/entity/registry.hpp>

static bool burn(entt::registry& registry, bim::game::arena& arena,
                 std::uint8_t x, std::uint8_t y,
                 bim::game::flame_horizontal horizontal,
                 bim::game::flame_vertical vertical, bim::game::flame_end end)
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
        x, y,
        bim::game::flame_factory(registry, x, y, horizontal, vertical, end));

  return true;
}

static void create_flames(entt::registry& registry, bim::game::arena& arena,
                          bim::game::position_on_grid p, std::uint8_t strength)
{
  // On the left.
  for (std::uint8_t offset = 1; offset <= strength; ++offset)
    if ((p.x < offset)
        || !burn(registry, arena, p.x - offset, p.y,
                 bim::game::flame_horizontal::yes,
                 bim::game::flame_vertical::no,
                 (offset == strength) ? bim::game::flame_end::yes
                                      : bim::game::flame_end::no))
      break;

  // On the right.
  for (std::uint8_t offset = 1, width = arena.width(); offset <= strength;
       ++offset)
    if ((p.x + offset == width)
        || !burn(registry, arena, p.x + offset, p.y,
                 bim::game::flame_horizontal::yes,
                 bim::game::flame_vertical::no,
                 (offset == strength) ? bim::game::flame_end::yes
                                      : bim::game::flame_end::no))
      break;

  // Above.
  for (std::uint8_t offset = 1; offset <= strength; ++offset)
    if ((p.y < offset)
        || !burn(registry, arena, p.x, p.y - offset,
                 bim::game::flame_horizontal::no,
                 bim::game::flame_vertical::yes,
                 (offset == strength) ? bim::game::flame_end::yes
                                      : bim::game::flame_end::no))
      break;

  // Below.
  for (std::uint8_t offset = 1, height = arena.height(); offset <= strength;
       ++offset)
    if ((p.y + offset == height)
        || !burn(registry, arena, p.x, p.y + offset,
                 bim::game::flame_horizontal::no,
                 bim::game::flame_vertical::yes,
                 (offset == strength) ? bim::game::flame_end::yes
                                      : bim::game::flame_end::no))
      break;

  arena.put_entity(p.x, p.y,
                   bim::game::flame_factory(registry, p.x, p.y,
                                            bim::game::flame_horizontal::yes,
                                            bim::game::flame_vertical::yes,
                                            bim::game::flame_end::no));
}

void bim::game::update_bombs(entt::registry& registry, arena& arena,
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
