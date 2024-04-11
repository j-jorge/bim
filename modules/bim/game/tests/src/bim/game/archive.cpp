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
#include <bim/game/input_archive.hpp>
#include <bim/game/output_archive.hpp>

#include <bim/game/component/bomb.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/flame_direction.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_direction.hpp>
#include <bim/game/component/position_on_grid.hpp>

#include <entt/entity/registry.hpp>
#include <entt/entity/snapshot.hpp>

#include <gtest/gtest.h>

TEST(bim_game_archive, pod_components)
{
  entt::registry registry;
  entt::entity entities[] = { registry.create(), registry.create(),
                              registry.create(), registry.create() };

  registry.emplace<bim::game::bomb>(entities[0], std::chrono::milliseconds(2),
                                    18);

  registry.emplace<bim::game::position_on_grid>(entities[1], 11, 22);
  registry.emplace<bim::game::player>(entities[1], 32,
                                      bim::game::player_direction::up, 4);

  registry.emplace<bim::game::flame>(
      entities[3], bim::game::flame_direction::left,
      bim::game::flame_segment::tip, std::chrono::milliseconds(40));

  bim::game::archive_storage bytes;
  bim::game::output_archive out(bytes);

  entt::snapshot(registry)
      .entities(out)
      .component<bim::game::bomb, bim::game::position_on_grid,
                 bim::game::player, bim::game::flame>(out);

  registry.clear();

  for (std::size_t i = 0; i != std::size(entities); ++i)
    EXPECT_FALSE(registry.valid(entities[i])) << "i=" << i;

  bim::game::input_archive in(&bytes[0]);
  entt::snapshot_loader(registry)
      .entities(in)
      .component<bim::game::bomb, bim::game::position_on_grid,
                 bim::game::player, bim::game::flame>(in);

  for (std::size_t i = 0; i != std::size(entities); ++i)
    EXPECT_TRUE(registry.valid(entities[i])) << "i=" << i;

  ASSERT_TRUE(registry.storage<bim::game::bomb>().contains(entities[0]));
  EXPECT_EQ(
      std::chrono::milliseconds(2),
      registry.get<bim::game::bomb>(entities[0]).duration_until_explosion);
  EXPECT_EQ(18, registry.get<bim::game::bomb>(entities[0]).strength);

  ASSERT_TRUE(
      registry.storage<bim::game::position_on_grid>().contains(entities[1]));
  EXPECT_EQ(11, registry.get<bim::game::position_on_grid>(entities[1]).x);
  EXPECT_EQ(22, registry.get<bim::game::position_on_grid>(entities[1]).y);

  ASSERT_TRUE(registry.storage<bim::game::player>().contains(entities[1]));
  EXPECT_EQ(32, registry.get<bim::game::player>(entities[1]).index);
  EXPECT_EQ(bim::game::player_direction::up,
            registry.get<bim::game::player>(entities[1]).current_direction);
  EXPECT_EQ(4, registry.get<bim::game::player>(entities[1]).bomb_strength);

  ASSERT_TRUE(registry.storage<bim::game::flame>().contains(entities[3]));
  EXPECT_EQ(bim::game::flame_direction::left,
            registry.get<bim::game::flame>(entities[3]).direction);
  EXPECT_EQ(bim::game::flame_segment::tip,
            registry.get<bim::game::flame>(entities[3]).segment);
  EXPECT_EQ(std::chrono::milliseconds(40),
            registry.get<bim::game::flame>(entities[3]).time_to_live);
}

TEST(bim_game_archive, pod_components_multiple_steps)
{
  entt::registry registry;
  entt::entity entities[] = { registry.create(), registry.create(),
                              registry.create(), registry.create() };

  registry.emplace<bim::game::bomb>(entities[0], std::chrono::milliseconds(2),
                                    18);

  registry.emplace<bim::game::position_on_grid>(entities[1], 11, 22);
  registry.emplace<bim::game::player>(entities[1], 24,
                                      bim::game::player_direction::up, 4);

  registry.emplace<bim::game::flame>(
      entities[3], bim::game::flame_direction::left,
      bim::game::flame_segment::tip, std::chrono::milliseconds(40));

  bim::game::archive_storage bytes;
  bim::game::output_archive out(bytes);

  entt::snapshot(registry)
      .entities(out)
      .component<bim::game::bomb>(out)
      .component<bim::game::position_on_grid>(out)
      .component<bim::game::player>(out)
      .component<bim::game::flame>(out);

  registry.clear();

  for (std::size_t i = 0; i != std::size(entities); ++i)
    EXPECT_FALSE(registry.valid(entities[i])) << "i=" << i;

  bim::game::input_archive in(&bytes[0]);
  // It is essential to keep the order indentical to above.
  entt::snapshot_loader(registry)
      .entities(in)
      .component<bim::game::bomb>(in)
      .component<bim::game::position_on_grid>(in)
      .component<bim::game::player>(in)
      .component<bim::game::flame>(in);

  for (std::size_t i = 0; i != std::size(entities); ++i)
    EXPECT_TRUE(registry.valid(entities[i])) << "i=" << i;

  ASSERT_TRUE(registry.storage<bim::game::bomb>().contains(entities[0]));
  EXPECT_EQ(
      std::chrono::milliseconds(2),
      registry.get<bim::game::bomb>(entities[0]).duration_until_explosion);
  EXPECT_EQ(18, registry.get<bim::game::bomb>(entities[0]).strength);

  ASSERT_TRUE(
      registry.storage<bim::game::position_on_grid>().contains(entities[1]));
  EXPECT_EQ(11, registry.get<bim::game::position_on_grid>(entities[1]).x);
  EXPECT_EQ(22, registry.get<bim::game::position_on_grid>(entities[1]).y);

  ASSERT_TRUE(registry.storage<bim::game::player>().contains(entities[1]));
  EXPECT_EQ(24, registry.get<bim::game::player>(entities[1]).index);
  EXPECT_EQ(bim::game::player_direction::up,
            registry.get<bim::game::player>(entities[1]).current_direction);
  EXPECT_EQ(4, registry.get<bim::game::player>(entities[1]).bomb_strength);

  ASSERT_TRUE(registry.storage<bim::game::flame>().contains(entities[3]));
  EXPECT_EQ(bim::game::flame_direction::left,
            registry.get<bim::game::flame>(entities[3]).direction);
  EXPECT_EQ(bim::game::flame_segment::tip,
            registry.get<bim::game::flame>(entities[3]).segment);
  EXPECT_EQ(std::chrono::milliseconds(40),
            registry.get<bim::game::flame>(entities[3]).time_to_live);
}
