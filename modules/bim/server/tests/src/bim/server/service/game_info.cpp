// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/game_info.hpp>

#include <gtest/gtest.h>

TEST(game_info, session_index)
{
  const bim::server::game_info game = { .sessions = { 2, 4, 8, 6 } };

  EXPECT_EQ(0, game.session_index(2));
  EXPECT_EQ(1, game.session_index(4));
  EXPECT_EQ(2, game.session_index(8));
  EXPECT_EQ(3, game.session_index(6));

  EXPECT_EQ(4, game.session_index(20));
  EXPECT_EQ(4, game.session_index(0));
}
