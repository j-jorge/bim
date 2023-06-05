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
#include <bm/net/message/game_update_from_server.hpp>

#include <bm/game/component/player_action_kind.hpp>

#include <gtest/gtest.h>

TEST(bm_net_game_update_from_server, serialization)
{
  bm::net::game_update_from_server source;

  EXPECT_EQ(bm::net::message_type::game_update_from_server, source.get_type());

  source.first_tick = 24;

  source.action_count.push_back(1);
  source.action_count.push_back(2);

  source.action_count.push_back(3);
  source.action_count.push_back(4);

  source.actions.push_back(bm::game::player_action_kind::drop_bomb);

  source.actions.push_back(bm::game::player_action_kind::drop_bomb);
  source.actions.push_back(bm::game::player_action_kind::up);

  source.actions.push_back(bm::game::player_action_kind::drop_bomb);
  source.actions.push_back(bm::game::player_action_kind::left);
  source.actions.push_back(bm::game::player_action_kind::drop_bomb);

  source.actions.push_back(bm::game::player_action_kind::up);
  source.actions.push_back(bm::game::player_action_kind::down);
  source.actions.push_back(bm::game::player_action_kind::left);
  source.actions.push_back(bm::game::player_action_kind::right);

  const iscool::net::message message(source.build_message());

  EXPECT_EQ(bm::net::message_type::game_update_from_server,
            message.get_type());

  const bm::net::game_update_from_server deserialized(message.get_content());

  EXPECT_EQ(24, deserialized.first_tick);

  ASSERT_EQ(4, deserialized.action_count.size());
  EXPECT_EQ(1, deserialized.action_count[0]);
  EXPECT_EQ(2, deserialized.action_count[1]);
  EXPECT_EQ(3, deserialized.action_count[2]);
  EXPECT_EQ(4, deserialized.action_count[3]);

  ASSERT_EQ(10, deserialized.actions.size());
  EXPECT_EQ(bm::game::player_action_kind::drop_bomb, deserialized.actions[0]);

  EXPECT_EQ(bm::game::player_action_kind::drop_bomb, deserialized.actions[1]);
  EXPECT_EQ(bm::game::player_action_kind::up, deserialized.actions[2]);

  EXPECT_EQ(bm::game::player_action_kind::drop_bomb, deserialized.actions[3]);
  EXPECT_EQ(bm::game::player_action_kind::left, deserialized.actions[4]);
  EXPECT_EQ(bm::game::player_action_kind::drop_bomb, deserialized.actions[5]);

  EXPECT_EQ(bm::game::player_action_kind::up, deserialized.actions[6]);
  EXPECT_EQ(bm::game::player_action_kind::down, deserialized.actions[7]);
  EXPECT_EQ(bm::game::player_action_kind::left, deserialized.actions[8]);
  EXPECT_EQ(bm::game::player_action_kind::right, deserialized.actions[9]);
}
