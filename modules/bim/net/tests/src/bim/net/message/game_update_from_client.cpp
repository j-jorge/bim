// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/net/message/game_update_from_client.hpp>

#include <bim/game/component/player_movement.hpp>

#include <gtest/gtest.h>

TEST(bim_net_game_update_from_client, serialization)
{
  bim::net::game_update_from_client source;

  EXPECT_EQ(bim::net::message_type::game_update_from_client,
            source.get_type());

  source.from_tick = 24;

  source.actions.push_back({ bim::game::player_movement::idle, false });
  source.actions.push_back({ bim::game::player_movement::left, false });
  source.actions.push_back({ bim::game::player_movement::right, false });
  source.actions.push_back({ bim::game::player_movement::up, false });
  source.actions.push_back({ bim::game::player_movement::down, false });

  source.actions.push_back({ bim::game::player_movement::idle, true });
  source.actions.push_back({ bim::game::player_movement::left, true });
  source.actions.push_back({ bim::game::player_movement::right, true });
  source.actions.push_back({ bim::game::player_movement::up, true });
  source.actions.push_back({ bim::game::player_movement::down, true });

  const iscool::net::message message(source.build_message());

  EXPECT_EQ(bim::net::message_type::game_update_from_client,
            message.get_type());

  const bim::net::game_update_from_client deserialized(message.get_content());

  EXPECT_EQ(24, deserialized.from_tick);

  ASSERT_EQ(10, deserialized.actions.size());

  EXPECT_EQ(bim::game::player_movement::idle,
            deserialized.actions[0].movement);
  EXPECT_FALSE(deserialized.actions[0].drop_bomb);

  EXPECT_EQ(bim::game::player_movement::left,
            deserialized.actions[1].movement);
  EXPECT_FALSE(deserialized.actions[1].drop_bomb);

  EXPECT_EQ(bim::game::player_movement::right,
            deserialized.actions[2].movement);
  EXPECT_FALSE(deserialized.actions[2].drop_bomb);

  EXPECT_EQ(bim::game::player_movement::up, deserialized.actions[3].movement);
  EXPECT_FALSE(deserialized.actions[3].drop_bomb);

  EXPECT_EQ(bim::game::player_movement::down,
            deserialized.actions[4].movement);
  EXPECT_FALSE(deserialized.actions[4].drop_bomb);

  EXPECT_EQ(bim::game::player_movement::idle,
            deserialized.actions[5].movement);
  EXPECT_TRUE(deserialized.actions[5].drop_bomb);

  EXPECT_EQ(bim::game::player_movement::left,
            deserialized.actions[6].movement);
  EXPECT_TRUE(deserialized.actions[6].drop_bomb);

  EXPECT_EQ(bim::game::player_movement::right,
            deserialized.actions[7].movement);
  EXPECT_TRUE(deserialized.actions[7].drop_bomb);

  EXPECT_EQ(bim::game::player_movement::up, deserialized.actions[8].movement);
  EXPECT_TRUE(deserialized.actions[8].drop_bomb);

  EXPECT_EQ(bim::game::player_movement::down,
            deserialized.actions[9].movement);
  EXPECT_TRUE(deserialized.actions[9].drop_bomb);
}
