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
#include <bm/game/input_archive.hpp>
#include <bm/game/output_archive.hpp>

#include <bm/game/component/bomb.hpp>
#include <bm/game/component/flame.hpp>
#include <bm/game/component/flame_direction.hpp>
#include <bm/game/component/player.hpp>
#include <bm/game/component/player_direction.hpp>
#include <bm/game/component/position_on_grid.hpp>

#include <entt/entity/registry.hpp>
#include <entt/entity/snapshot.hpp>

#include <gtest/gtest.h>

TEST(bm_game_archive, pod_components)
{
  entt::registry registry;
  entt::entity entities[] = { registry.create(), registry.create(),
                              registry.create(), registry.create() };

  registry.emplace<bm::game::bomb>(entities[0], std::chrono::milliseconds(2),
                                   18);

  registry.emplace<bm::game::position_on_grid>(entities[1], 11, 22);
  registry.emplace<bm::game::player>(entities[1], 32,
                                     bm::game::player_direction::up, 4);

  registry.emplace<bm::game::flame>(
      entities[3], bm::game::flame_horizontal::yes,
      bm::game::flame_vertical::no, bm::game::flame_end::yes,
      std::chrono::milliseconds(40));

  bm::game::archive_storage bytes;
  bm::game::output_archive out(bytes);

  entt::snapshot(registry)
      .entities(out)
      .component<bm::game::bomb, bm::game::position_on_grid, bm::game::player,
                 bm::game::flame>(out);

  registry.clear();

  for (std::size_t i = 0; i != std::size(entities); ++i)
    EXPECT_FALSE(registry.valid(entities[i])) << "i=" << i;

  bm::game::input_archive in(&bytes[0]);
  entt::snapshot_loader(registry)
      .entities(in)
      .component<bm::game::bomb, bm::game::position_on_grid, bm::game::player,
                 bm::game::flame>(in);

  for (std::size_t i = 0; i != std::size(entities); ++i)
    EXPECT_TRUE(registry.valid(entities[i])) << "i=" << i;

  ASSERT_TRUE(registry.storage<bm::game::bomb>().contains(entities[0]));
  EXPECT_EQ(
      std::chrono::milliseconds(2),
      registry.get<bm::game::bomb>(entities[0]).duration_until_explosion);
  EXPECT_EQ(18, registry.get<bm::game::bomb>(entities[0]).strength);

  ASSERT_TRUE(
      registry.storage<bm::game::position_on_grid>().contains(entities[1]));
  EXPECT_EQ(11, registry.get<bm::game::position_on_grid>(entities[1]).x);
  EXPECT_EQ(22, registry.get<bm::game::position_on_grid>(entities[1]).y);

  ASSERT_TRUE(registry.storage<bm::game::player>().contains(entities[1]));
  EXPECT_EQ(32, registry.get<bm::game::player>(entities[1]).index);
  EXPECT_EQ(bm::game::player_direction::up,
            registry.get<bm::game::player>(entities[1]).current_direction);
  EXPECT_EQ(4, registry.get<bm::game::player>(entities[1]).bomb_strength);

  ASSERT_TRUE(registry.storage<bm::game::flame>().contains(entities[3]));
  EXPECT_EQ(bm::game::flame_horizontal::yes,
            registry.get<bm::game::flame>(entities[3]).horizontal);
  EXPECT_EQ(bm::game::flame_vertical::no,
            registry.get<bm::game::flame>(entities[3]).vertical);
  EXPECT_EQ(bm::game::flame_end::yes,
            registry.get<bm::game::flame>(entities[3]).end);
  EXPECT_EQ(std::chrono::milliseconds(40),
            registry.get<bm::game::flame>(entities[3]).time_to_live);
}

TEST(bm_game_archive, pod_components_multiple_steps)
{
  entt::registry registry;
  entt::entity entities[] = { registry.create(), registry.create(),
                              registry.create(), registry.create() };

  registry.emplace<bm::game::bomb>(entities[0], std::chrono::milliseconds(2),
                                   18);

  registry.emplace<bm::game::position_on_grid>(entities[1], 11, 22);
  registry.emplace<bm::game::player>(entities[1], 24,
                                     bm::game::player_direction::up, 4);

  registry.emplace<bm::game::flame>(
      entities[3], bm::game::flame_horizontal::yes,
      bm::game::flame_vertical::no, bm::game::flame_end::yes,
      std::chrono::milliseconds(40));

  bm::game::archive_storage bytes;
  bm::game::output_archive out(bytes);

  entt::snapshot(registry)
      .entities(out)
      .component<bm::game::bomb>(out)
      .component<bm::game::position_on_grid>(out)
      .component<bm::game::player>(out)
      .component<bm::game::flame>(out);

  registry.clear();

  for (std::size_t i = 0; i != std::size(entities); ++i)
    EXPECT_FALSE(registry.valid(entities[i])) << "i=" << i;

  bm::game::input_archive in(&bytes[0]);
  // It is essential to keep the order indentical to above.
  entt::snapshot_loader(registry)
      .entities(in)
      .component<bm::game::bomb>(in)
      .component<bm::game::position_on_grid>(in)
      .component<bm::game::player>(in)
      .component<bm::game::flame>(in);

  for (std::size_t i = 0; i != std::size(entities); ++i)
    EXPECT_TRUE(registry.valid(entities[i])) << "i=" << i;

  ASSERT_TRUE(registry.storage<bm::game::bomb>().contains(entities[0]));
  EXPECT_EQ(
      std::chrono::milliseconds(2),
      registry.get<bm::game::bomb>(entities[0]).duration_until_explosion);
  EXPECT_EQ(18, registry.get<bm::game::bomb>(entities[0]).strength);

  ASSERT_TRUE(
      registry.storage<bm::game::position_on_grid>().contains(entities[1]));
  EXPECT_EQ(11, registry.get<bm::game::position_on_grid>(entities[1]).x);
  EXPECT_EQ(22, registry.get<bm::game::position_on_grid>(entities[1]).y);

  ASSERT_TRUE(registry.storage<bm::game::player>().contains(entities[1]));
  EXPECT_EQ(24, registry.get<bm::game::player>(entities[1]).index);
  EXPECT_EQ(bm::game::player_direction::up,
            registry.get<bm::game::player>(entities[1]).current_direction);
  EXPECT_EQ(4, registry.get<bm::game::player>(entities[1]).bomb_strength);

  ASSERT_TRUE(registry.storage<bm::game::flame>().contains(entities[3]));
  EXPECT_EQ(bm::game::flame_horizontal::yes,
            registry.get<bm::game::flame>(entities[3]).horizontal);
  EXPECT_EQ(bm::game::flame_vertical::no,
            registry.get<bm::game::flame>(entities[3]).vertical);
  EXPECT_EQ(bm::game::flame_end::yes,
            registry.get<bm::game::flame>(entities[3]).end);
  EXPECT_EQ(std::chrono::milliseconds(40),
            registry.get<bm::game::flame>(entities[3]).time_to_live);
}
