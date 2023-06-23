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
#include <bim/server/service/game_service.hpp>

#include <bim/server/service/game_info.hpp>

#include <iscool/net/socket_stream.h>

#include <gtest/gtest.h>

TEST(game_service, new_game)
{
  iscool::net::socket_stream socket_stream(12345);
  bim::server::game_service service(socket_stream);

  const bim::server::game_info game = service.new_game(4, { 11, 22, 33, 44 });

  EXPECT_EQ(4, game.player_count);
  EXPECT_EQ(11, game.sessions[0]);
  EXPECT_EQ(22, game.sessions[1]);
  EXPECT_EQ(33, game.sessions[2]);
  EXPECT_EQ(44, game.sessions[3]);

  EXPECT_NE(44, game.channel);
  EXPECT_FALSE(!!service.find_game(44));

  const std::optional<bim::server::game_info> game_opt =
      service.find_game(game.channel);

  EXPECT_TRUE(!!game_opt);

  EXPECT_EQ(4, game_opt->player_count);
  EXPECT_EQ(11, game_opt->sessions[0]);
  EXPECT_EQ(22, game_opt->sessions[1]);
  EXPECT_EQ(33, game_opt->sessions[2]);
  EXPECT_EQ(44, game_opt->sessions[3]);
}
