// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/net/message/hello_ok.hpp>

#include <gtest/gtest.h>

TEST(bim_net_hello_ok, serialization)
{
  bim::net::hello_ok source;

  EXPECT_EQ(bim::net::message_type::hello_ok, source.get_type());

  source.request_token = 1;
  source.version = 2;
  source.games_now = 3;
  source.games_last_hour = 4;
  source.games_last_day = 5;
  source.games_last_month = 6;
  source.sessions_now = 7;
  source.sessions_last_hour = 8;
  source.sessions_last_day = 9;
  source.sessions_last_month = 10;
  source.name = "foo";

  iscool::net::message message;
  source.build_message(message);
  EXPECT_EQ(bim::net::message_type::hello_ok, message.get_type());

  const bim::net::hello_ok deserialized(message.get_content());

  EXPECT_EQ(1, source.request_token);
  EXPECT_EQ(2, source.version);
  EXPECT_EQ(3, source.games_now);
  EXPECT_EQ(4, source.games_last_hour);
  EXPECT_EQ(5, source.games_last_day);
  EXPECT_EQ(6, source.games_last_month);
  EXPECT_EQ(7, source.sessions_now);
  EXPECT_EQ(8, source.sessions_last_hour);
  EXPECT_EQ(9, source.sessions_last_day);
  EXPECT_EQ(10, source.sessions_last_month);
  EXPECT_EQ("foo", source.name);
}

TEST(bim_net_hello_ok, null_byte_in_name)
{
  bim::net::hello_ok source;

  EXPECT_EQ(bim::net::message_type::hello_ok, source.get_type());

  source.name = std::string("foo\0bar", 7);

  iscool::net::message message;
  source.build_message(message);

  const bim::net::hello_ok deserialized(message.get_content());

  EXPECT_EQ(7, source.name.size());
  EXPECT_EQ(std::string("foo\0bar", 7), source.name);
}
