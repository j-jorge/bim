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
#include <bim/net/message/game_update_from_server.hpp>

#include <bim/game/component/player_movement.hpp>

#include <gtest/gtest.h>

TEST(bim_net_game_update_from_server, serialization)
{
  bim::net::game_update_from_server source;

  EXPECT_EQ(bim::net::message_type::game_update_from_server,
            source.get_type());

  source.from_tick = 24;

  source.actions.resize(4);

  source.actions[0].push_back({ bim::game::player_movement::idle, false });
  source.actions[0].push_back({ bim::game::player_movement::left, false });
  source.actions[0].push_back({ bim::game::player_movement::right, false });
  source.actions[1].push_back({ bim::game::player_movement::up, false });
  source.actions[1].push_back({ bim::game::player_movement::down, false });

  source.actions[1].push_back({ bim::game::player_movement::idle, true });
  source.actions[2].push_back({ bim::game::player_movement::left, true });
  source.actions[2].push_back({ bim::game::player_movement::right, true });
  source.actions[2].push_back({ bim::game::player_movement::up, true });
  source.actions[3].push_back({ bim::game::player_movement::down, true });
  source.actions[3].push_back({ bim::game::player_movement::down, true });
  source.actions[3].push_back({ bim::game::player_movement::down, true });

  const iscool::net::message message(source.build_message());

  EXPECT_EQ(bim::net::message_type::game_update_from_server,
            message.get_type());

  const bim::net::game_update_from_server deserialized(message.get_content());

  EXPECT_EQ(24, deserialized.from_tick);

  ASSERT_EQ(4, deserialized.actions.size());

  ASSERT_EQ(3, deserialized.actions[0].size());
  EXPECT_EQ(bim::game::player_movement::idle,
            deserialized.actions[0][0].movement);
  EXPECT_FALSE(deserialized.actions[0][0].drop_bomb);

  EXPECT_EQ(bim::game::player_movement::left,
            deserialized.actions[0][1].movement);
  EXPECT_FALSE(deserialized.actions[0][1].drop_bomb);

  EXPECT_EQ(bim::game::player_movement::right,
            deserialized.actions[0][2].movement);
  EXPECT_FALSE(deserialized.actions[0][2].drop_bomb);

  ASSERT_EQ(3, deserialized.actions[1].size());
  EXPECT_EQ(bim::game::player_movement::up,
            deserialized.actions[1][0].movement);
  EXPECT_FALSE(deserialized.actions[1][0].drop_bomb);

  EXPECT_EQ(bim::game::player_movement::down,
            deserialized.actions[1][1].movement);
  EXPECT_FALSE(deserialized.actions[1][1].drop_bomb);

  EXPECT_EQ(bim::game::player_movement::idle,
            deserialized.actions[1][2].movement);
  EXPECT_TRUE(deserialized.actions[1][2].drop_bomb);

  ASSERT_EQ(3, deserialized.actions[2].size());
  EXPECT_EQ(bim::game::player_movement::left,
            deserialized.actions[2][0].movement);
  EXPECT_TRUE(deserialized.actions[2][0].drop_bomb);

  EXPECT_EQ(bim::game::player_movement::right,
            deserialized.actions[2][1].movement);
  EXPECT_TRUE(deserialized.actions[2][1].drop_bomb);

  EXPECT_EQ(bim::game::player_movement::up,
            deserialized.actions[2][2].movement);
  EXPECT_TRUE(deserialized.actions[2][2].drop_bomb);

  ASSERT_EQ(3, deserialized.actions[3].size());
  EXPECT_EQ(bim::game::player_movement::down,
            deserialized.actions[3][0].movement);
  EXPECT_TRUE(deserialized.actions[3][0].drop_bomb);

  EXPECT_EQ(bim::game::player_movement::down,
            deserialized.actions[3][1].movement);
  EXPECT_TRUE(deserialized.actions[3][1].drop_bomb);

  EXPECT_EQ(bim::game::player_movement::down,
            deserialized.actions[3][2].movement);
  EXPECT_TRUE(deserialized.actions[3][2].drop_bomb);
}
