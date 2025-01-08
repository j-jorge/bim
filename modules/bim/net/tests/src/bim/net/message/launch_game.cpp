// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/net/message/launch_game.hpp>

#include <gtest/gtest.h>

TEST(bim_net_launch_game, serialization)
{
  bim::net::launch_game source(0x1122334455667788, 0xfedcba9876543210,
                               0x11111111, 0x22222222, 4, 3, 0xfe, 0xef, 0xff);

  EXPECT_EQ(bim::net::message_type::launch_game, source.get_type());

  iscool::net::message message;
  source.build_message(message);
  EXPECT_EQ(bim::net::message_type::launch_game, message.get_type());

  const bim::net::launch_game deserialized(message.get_content());

  EXPECT_EQ(0x1122334455667788, deserialized.get_request_token());
  EXPECT_EQ(0xfedcba9876543210, deserialized.get_seed());
  EXPECT_EQ(0x11111111, deserialized.get_game_channel());
  EXPECT_EQ(0x22222222, deserialized.get_feature_mask());
  EXPECT_EQ(4, deserialized.get_player_count());
  EXPECT_EQ(3, deserialized.get_player_index());
  EXPECT_EQ(0xfe, deserialized.get_brick_wall_probability());
  EXPECT_EQ(0xef, deserialized.get_arena_width());
  EXPECT_EQ(0xff, deserialized.get_arena_height());
}
