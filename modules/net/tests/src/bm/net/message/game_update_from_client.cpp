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
#include <bm/net/message/game_update_from_client.hpp>

#include <bm/game/component/player_action_kind.hpp>

#include <gtest/gtest.h>

TEST(bm_net_game_update_from_client, serialization)
{
  bm::net::game_update_from_client source;

  EXPECT_EQ(bm::net::message_type::game_update_from_client, source.get_type());

  source.from_tick = 24;

  source.action_count_at_tick.push_back(1);
  source.action_count_at_tick.push_back(0);
  source.action_count_at_tick.push_back(3);

  source.actions.push_back(bm::game::player_action_kind::drop_bomb);
  source.actions.push_back(bm::game::player_action_kind::right);
  source.actions.push_back(bm::game::player_action_kind::up);
  source.actions.push_back(bm::game::player_action_kind::drop_bomb);
  source.actions.push_back(bm::game::player_action_kind::left);

  const iscool::net::message message(source.build_message());

  EXPECT_EQ(bm::net::message_type::game_update_from_client,
            message.get_type());

  const bm::net::game_update_from_client deserialized(message.get_content());

  EXPECT_EQ(24, deserialized.from_tick);

  ASSERT_EQ(3, deserialized.action_count_at_tick.size());
  EXPECT_EQ(1, deserialized.action_count_at_tick[0]);
  EXPECT_EQ(0, deserialized.action_count_at_tick[1]);
  EXPECT_EQ(3, deserialized.action_count_at_tick[2]);

  ASSERT_EQ(5, deserialized.actions.size());
  EXPECT_EQ(bm::game::player_action_kind::drop_bomb, deserialized.actions[0]);
  EXPECT_EQ(bm::game::player_action_kind::right, deserialized.actions[1]);
  EXPECT_EQ(bm::game::player_action_kind::up, deserialized.actions[2]);
  EXPECT_EQ(bm::game::player_action_kind::drop_bomb, deserialized.actions[3]);
  EXPECT_EQ(bm::game::player_action_kind::left, deserialized.actions[4]);
}
