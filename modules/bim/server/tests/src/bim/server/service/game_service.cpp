// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/fake_scheduler.hpp>

#include <bim/server/config.hpp>
#include <bim/server/service/game_info.hpp>
#include <bim/server/service/game_service.hpp>
#include <bim/server/service/server_stats.hpp>

#include <iscool/net/socket_stream.hpp>

#include <gtest/gtest.h>

TEST(game_service, new_game)
{
  bim::server::tests::fake_scheduler scheduler;
  bim::server::server_stats server_stats(std::chrono::minutes(0),
                                         std::chrono::days(0), true);

  iscool::net::socket_stream socket_stream(12345);
  bim::server::game_service service({}, socket_stream, server_stats);
  const bim::game::feature_flags features = (bim::game::feature_flags)42;

  const bim::server::game_info game =
      service.new_game(4, features, { 11, 22, 33, 44 });

  EXPECT_EQ(4, game.fingerprint.player_count);
  EXPECT_EQ(features, game.fingerprint.features);
  EXPECT_EQ(11, game.sessions[0]);
  EXPECT_EQ(22, game.sessions[1]);
  EXPECT_EQ(33, game.sessions[2]);
  EXPECT_EQ(44, game.sessions[3]);

  EXPECT_NE(44, game.channel);
  EXPECT_FALSE(!!service.find_game(44));

  std::optional<bim::server::game_info> game_opt =
      service.find_game(game.channel);

  ASSERT_TRUE(!!game_opt);

  EXPECT_EQ(4, game_opt->fingerprint.player_count);
  EXPECT_EQ(features, game_opt->fingerprint.features);
  EXPECT_EQ(11, game_opt->sessions[0]);
  EXPECT_EQ(22, game_opt->sessions[1]);
  EXPECT_EQ(33, game_opt->sessions[2]);
  EXPECT_EQ(44, game_opt->sessions[3]);

  std::this_thread::sleep_for(std::chrono::seconds(0));
  scheduler.tick(std::chrono::minutes(10));

  game_opt = service.find_game(game.channel);

  EXPECT_FALSE(!!game_opt);
}
