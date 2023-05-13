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
#include <bm/game/system/update_flames.hpp>

#include <bm/game/arena.hpp>

#include <bm/game/component/flame.hpp>
#include <bm/game/component/flame_direction.hpp>
#include <bm/game/factory/flame.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(update_flames, time_to_live)
{
  entt::registry registry;
  bm::game::arena arena(3, 3);

  const entt::entity entity = bm::game::flame_factory(
      registry, 0, 0, bm::game::flame_horizontal::yes,
      bm::game::flame_vertical::yes, bm::game::flame_end::no,
      std::chrono::milliseconds(24));
  bm::game::flame& flame = registry.get<bm::game::flame>(entity);

  EXPECT_EQ(std::chrono::milliseconds(24), flame.time_to_live);

  bm::game::update_flames(registry, arena, std::chrono::milliseconds(12));
  EXPECT_EQ(std::chrono::milliseconds(12), flame.time_to_live);
}
