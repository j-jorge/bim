#include <bim/game/component/player_action.hpp>

#include <bim/game/component/player_action_kind.hpp>

#include <gtest/gtest.h>

TEST(bim_game_player_action, push)
{
  bim::game::player_action action{};

  ASSERT_EQ(0, action.queue_size);

  action.push(bim::game::player_action_kind::up);
  action.push(bim::game::player_action_kind::drop_bomb);
  action.push(bim::game::player_action_kind::left);

  EXPECT_EQ(3, action.queue_size);
  EXPECT_EQ(bim::game::player_action_kind::up, action.queue[0]);
  EXPECT_EQ(bim::game::player_action_kind::drop_bomb, action.queue[1]);
  EXPECT_EQ(bim::game::player_action_kind::left, action.queue[2]);
}
