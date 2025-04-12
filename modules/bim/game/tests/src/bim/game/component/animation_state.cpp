// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/component/animation_state.hpp>

#include <gtest/gtest.h>

TEST(bim_game_animation_state, reset_duration_on_actual_transition)
{
  bim::game::animation_state state;
  state.model = bim::game::animation_id(24);
  state.elapsed_time = std::chrono::milliseconds(11);

  // Transition to the same animation: no effect.
  state.transition_to(bim::game::animation_id(24));
  EXPECT_EQ(std::chrono::milliseconds(11), state.elapsed_time);

  // Transition to a different animation: reset the elapsed time.
  state.transition_to(bim::game::animation_id(42));
  EXPECT_EQ(std::chrono::milliseconds(0), state.elapsed_time);
}
