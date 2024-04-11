#include <bim/game/component/flame_direction.hpp>

#include <gtest/gtest.h>

TEST(bim_game_flame_direction, horizontal_or_vertical)
{
  EXPECT_TRUE(bim::game::is_horizontal(bim::game::flame_direction::left));
  EXPECT_FALSE(bim::game::is_vertical(bim::game::flame_direction::left));

  EXPECT_TRUE(bim::game::is_horizontal(bim::game::flame_direction::right));
  EXPECT_FALSE(bim::game::is_vertical(bim::game::flame_direction::right));

  EXPECT_FALSE(bim::game::is_horizontal(bim::game::flame_direction::up));
  EXPECT_TRUE(bim::game::is_vertical(bim::game::flame_direction::up));

  EXPECT_FALSE(bim::game::is_horizontal(bim::game::flame_direction::down));
  EXPECT_TRUE(bim::game::is_vertical(bim::game::flame_direction::down));
}
