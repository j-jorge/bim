#include <bim/game/component/player_action_queue.hpp>

#include <bim/game/component/player_movement.hpp>

#include <gtest/gtest.h>

TEST(bim_game_player_action_queue, enqueue)
{
  bim::game::player_action_queue queue{};

  const bim::game::player_action action_in{ bim::game::player_movement::up,
                                            true };

  bim::game::player_action action_out = queue.enqueue(action_in);

  for (int i = 0; i != bim::game::player_action_queue::queue_size; ++i)
    {
      EXPECT_EQ(bim::game::player_movement::idle, action_out.movement)
          << "i=" << i;
      EXPECT_FALSE(action_out.drop_bomb) << "i=" << i;
      action_out = queue.enqueue({});
    }

  EXPECT_EQ(action_in.movement, action_out.movement);
  EXPECT_EQ(action_in.drop_bomb, action_out.drop_bomb);
}
