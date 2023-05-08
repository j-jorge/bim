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
#include <bm/game/system/update_bombs.hpp>

#include <bm/game/arena.hpp>

#include <bm/game/component/bomb.hpp>
#include <bm/game/component/flame.hpp>
#include <bm/game/component/flame_direction.hpp>
#include <bm/game/component/position_on_grid.hpp>

#include <entt/entity/registry.hpp>

#include <string>
#include <vector>

#include <gtest/gtest.h>

static std::vector<std::string> flames_map(const bm::game::arena& arena,
                                           const entt::registry& registry)
{
  std::vector<std::string> result(arena.height(),
                                  std::string(arena.width(), ' '));

  registry.view<bm::game::flame, bm::game::position_on_grid>().each(
      [&](entt::entity e, bm::game::flame f,
          bm::game::position_on_grid p) -> void
      {
        EXPECT_TRUE(arena.entity_at(p.x, p.y) == e);
        EXPECT_EQ(' ', result[p.y][p.x]);

        EXPECT_TRUE((f.horizontal == bm::game::flame_horizontal::yes)
                    || (f.vertical == bm::game::flame_vertical::yes));

        if (f.horizontal == bm::game::flame_horizontal::yes)
          if (f.vertical == bm::game::flame_vertical::yes)
            result[p.y][p.x] = 'B';
          else if (f.end == bm::game::flame_end::yes)
            result[p.y][p.x] = 'h';
          else
            result[p.y][p.x] = 'H';
        else if (f.end == bm::game::flame_end::yes)
          result[p.y][p.x] = 'v';
        else
          result[p.y][p.x] = 'V';
      });

  return result;
}

TEST(update_bombs, delay)
{
  entt::registry registry;
  bm::game::arena arena(3, 3);

  const entt::entity entity = registry.create();
  const bm::game::bomb& bomb = registry.emplace<bm::game::bomb>(
      entity, std::chrono::milliseconds(24), 0);
  registry.emplace<bm::game::position_on_grid>(entity, 0, 0);

  EXPECT_EQ(std::chrono::milliseconds(24), bomb.duration_until_explosion);

  bm::game::update_bombs(registry, arena, std::chrono::milliseconds(12));
  EXPECT_EQ(std::chrono::milliseconds(12), bomb.duration_until_explosion);
}

TEST(update_bombs, explode_strength_2)
{
  entt::registry registry;
  bm::game::arena arena(5, 3);
  const std::uint8_t bomb_x = arena.width() / 2;
  const std::uint8_t bomb_y = arena.height() / 2;

  const entt::entity entity = registry.create();
  registry.emplace<bm::game::bomb>(entity, std::chrono::milliseconds(24), 2);
  registry.emplace<bm::game::position_on_grid>(entity, bomb_x, bomb_y);

  bm::game::update_bombs(registry, arena, std::chrono::milliseconds(12));
  EXPECT_TRUE(registry.storage<bm::game::bomb>().contains(entity));

  bm::game::update_bombs(registry, arena, std::chrono::milliseconds(12));
  EXPECT_FALSE(registry.storage<bm::game::bomb>().contains(entity));

  const std::vector<std::string> flames = flames_map(arena, registry);
  EXPECT_EQ("  V  ", flames[0]);
  EXPECT_EQ("hHBHh", flames[1]);
  EXPECT_EQ("  V  ", flames[2]);
}
