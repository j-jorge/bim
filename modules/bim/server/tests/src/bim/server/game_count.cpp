// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/client_server_simulator.hpp>

#include <bim/net/exchange/hello_exchange.hpp>
#include <bim/net/message/hello_ok.hpp>

#include <bim/server/tests/new_test_config.hpp>

#include <gtest/gtest.h>

TEST(game_count_test, game_started_and_game_over)
{
  bim::server::tests::client_server_simulator simulator(
      2, bim::server::tests::new_test_config());

  bim::net::hello_exchange hello(simulator.message_stream,
                                 std::chrono::seconds(0));

  std::optional<bim::net::hello_ok> answer_ok;

  hello.connect_to_updated(
      [&](bim::net::hello_ok h)
        {
          answer_ok = std::move(h);
          hello.stop();
        });

  // Check that no game is running.
  hello.start();

  simulator.wait(
      [&]()
        {
          return !!answer_ok;
        });

  ASSERT_TRUE(!!answer_ok);
  EXPECT_EQ(0, answer_ok->games_now);

  // Launch a game and join it.
  simulator.authenticate();
  simulator.join_game();

  // Check that one game is running.
  answer_ok = std::nullopt;
  hello.start();

  simulator.wait(
      [&]()
        {
          return !!answer_ok;
        });

  ASSERT_TRUE(!!answer_ok);
  EXPECT_EQ(1, answer_ok->games_now);

  // Now we terminate the game.
  simulator.drop_bombs(1);
  simulator.wait_game_over();

  // Check that no game is running.
  answer_ok = std::nullopt;
  hello.start();

  simulator.wait(
      [&]()
        {
          return !!answer_ok;
        });

  ASSERT_TRUE(!!answer_ok);
  EXPECT_EQ(0, answer_ok->games_now);
}

TEST(game_count_test, game_dropped)
{
  bim::server::config config = bim::server::tests::new_test_config();
  config.game_service_clean_up_interval = std::chrono::seconds(10);

  bim::server::tests::client_server_simulator simulator(2, config);

  bim::net::hello_exchange hello(simulator.message_stream,
                                 std::chrono::seconds(0));

  std::optional<bim::net::hello_ok> answer_ok;

  hello.connect_to_updated(
      [&](bim::net::hello_ok h)
        {
          answer_ok = std::move(h);
          hello.stop();
        });

  // Check that no game is running.
  hello.start();

  simulator.wait(
      [&]()
        {
          return !!answer_ok;
        });

  ASSERT_TRUE(!!answer_ok);
  EXPECT_EQ(0, answer_ok->games_now);

  // Launch a game and join it.
  simulator.authenticate();
  simulator.join_game();

  // Check that one game is running.
  answer_ok = std::nullopt;
  hello.start();

  simulator.wait(
      [&]()
        {
          return !!answer_ok;
        });

  ASSERT_TRUE(!!answer_ok);
  EXPECT_EQ(1, answer_ok->games_now);

  // Everybody leaves the game, ther will be no game over.
  simulator.clients[0].leave_game();
  simulator.clients[1].leave_game();

  simulator.wait(config.game_service_clean_up_interval
                 - std::chrono::seconds(1));

  // Check that no game is running.
  answer_ok = std::nullopt;
  hello.start();

  simulator.wait(
      [&]()
        {
          return !!answer_ok;
        });

  ASSERT_TRUE(!!answer_ok);
  EXPECT_EQ(1, answer_ok->games_now);

  simulator.wait(config.game_service_clean_up_interval);

  // Check that no game is running.
  answer_ok = std::nullopt;
  hello.start();

  simulator.wait(
      [&]()
        {
          return !!answer_ok;
        });

  ASSERT_TRUE(!!answer_ok);
  EXPECT_EQ(0, answer_ok->games_now);
}
