// SPDX-License-Identifier: AGPL-3.0-only
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
