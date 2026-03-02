// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/game_state_serialization.hpp>

#include <bim/game/component/bomb.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/flame_direction.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/timer.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(bim_game_game_state_serialization, pod_components_multiple_steps)
{
  entt::registry registry;
  entt::entity entities[] = { registry.create(), registry.create(),
                              registry.create(), registry.create() };

  registry.emplace<bim::game::bomb>(entities[0], 18);
  registry.emplace<bim::game::timer>(entities[0],
                                     std::chrono::milliseconds(2));

  registry.emplace<bim::game::position_on_grid>(entities[1], 11, 22);
  registry.emplace<bim::game::player>(entities[1], 24, 0, 0, 4);

  registry.emplace<bim::game::flame>(entities[3],
                                     bim::game::flame_direction::left,
                                     bim::game::flame_segment::tip);

  bim::game::archive_storage bytes;
  bim::game::serialize_state(bytes, registry);

  registry.clear();

  for (std::size_t i = 0; i != std::size(entities); ++i)
    EXPECT_FALSE(registry.valid(entities[i])) << "i=" << i;

  bim::game::deserialize_state(registry, bytes);

  for (std::size_t i = 0; i != std::size(entities); ++i)
    EXPECT_TRUE(registry.valid(entities[i])) << "i=" << i;

  ASSERT_TRUE(registry.storage<bim::game::bomb>().contains(entities[0]));
  EXPECT_EQ(18, registry.get<bim::game::bomb>(entities[0]).strength);

  ASSERT_TRUE(registry.storage<bim::game::timer>().contains(entities[0]));
  EXPECT_EQ(std::chrono::milliseconds(2),
            registry.get<bim::game::timer>(entities[0]).duration);

  ASSERT_TRUE(
      registry.storage<bim::game::position_on_grid>().contains(entities[1]));
  EXPECT_EQ(11, registry.get<bim::game::position_on_grid>(entities[1]).x);
  EXPECT_EQ(22, registry.get<bim::game::position_on_grid>(entities[1]).y);

  ASSERT_TRUE(registry.storage<bim::game::player>().contains(entities[1]));
  EXPECT_EQ(24, registry.get<bim::game::player>(entities[1]).index);
  EXPECT_EQ(4, registry.get<bim::game::player>(entities[1]).bomb_strength);

  ASSERT_TRUE(registry.storage<bim::game::flame>().contains(entities[3]));
  EXPECT_EQ(bim::game::flame_direction::left,
            registry.get<bim::game::flame>(entities[3]).direction);
  EXPECT_EQ(bim::game::flame_segment::tip,
            registry.get<bim::game::flame>(entities[3]).segment);
}
