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
#include <bm/server/service/game_info.hpp>

#include <gtest/gtest.h>

TEST(game_info, session_index)
{
  const bm::server::game_info game = { .sessions = { 2, 4, 8, 6 } };

  EXPECT_EQ(0, game.session_index(2));
  EXPECT_EQ(1, game.session_index(4));
  EXPECT_EQ(2, game.session_index(8));
  EXPECT_EQ(3, game.session_index(6));

  EXPECT_EQ(4, game.session_index(20));
  EXPECT_EQ(4, game.session_index(0));
}
