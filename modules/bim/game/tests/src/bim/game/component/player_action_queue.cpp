#include <bim/game/component/player_action_queue.hpp>

#include <bim/game/component/player_movement.hpp>

#include <gtest/gtest.h>

TEST(bim_game_player_action_queue, enqueue)
{
  bim::game::player_action_queue queue{};

  const bim::game::player_action action_in{ bim::game::player_movement::up,
                                            true };

  bim::game::queued_action action_out = queue.enqueue(action_in, 1, 2);

  for (int i = 0; i != bim::game::player_action_queue::queue_size; ++i)
    {
      EXPECT_EQ(bim::game::player_movement::idle, action_out.action.movement)
          << "i=" << i;
      EXPECT_FALSE(action_out.action.drop_bomb) << "i=" << i;
      EXPECT_EQ(0, action_out.arena_x) << "i=" << i;
      EXPECT_EQ(0, action_out.arena_y) << "i=" << i;

      action_out = queue.enqueue({}, 0, 0);
    }

  EXPECT_EQ(action_in.movement, action_out.action.movement);
  EXPECT_EQ(action_in.drop_bomb, action_out.action.drop_bomb);
  EXPECT_EQ(1, action_out.arena_x);
  EXPECT_EQ(2, action_out.arena_y);
}
