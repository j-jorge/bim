// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/test_client.hpp>

#include <bim/server/tests/fake_scheduler.hpp>
#include <bim/server/tests/new_test_config.hpp>

#include <bim/server/server.hpp>

#include <gtest/gtest.h>

class server_bots_test : public testing::Test
{
public:
  server_bots_test();

protected:
  bim::server::tests::fake_scheduler m_scheduler;

  const bim::server::config m_config;
  bim::server::server m_server;
  iscool::net::socket_stream m_socket_stream;
  iscool::net::message_stream m_message_stream;

  std::array<bim::server::tests::test_client, 2> m_clients;
};

server_bots_test::server_bots_test()
  : m_config(
        []()
          {
            bim::server::config config = bim::server::tests::new_test_config();

            config.enable_bots = true;
            config.matchmaking_delay_for_bot = std::chrono::seconds(1);

            return config;
          }())
  , m_server(m_config)
  , m_socket_stream("localhost:" + std::to_string(m_config.port),
                    iscool::net::socket_mode::client{})
  , m_message_stream(m_socket_stream)
  , m_clients{ bim::server::tests::test_client(m_scheduler, m_message_stream),
               bim::server::tests::test_client(m_scheduler, m_message_stream) }
{}

TEST_F(server_bots_test, start_with_a_bot)
{
  m_clients[0].authenticate();
  m_clients[1].authenticate();

  // Start a game with only the first player. A bot should be provided by the
  // server.
  m_clients[0].new_game_auto_accept();

  // Let the time pass such that the messages can move between the clients and
  // the server.
  for (int i = 0; i != 10; ++i)
    {
      m_scheduler.tick(std::chrono::seconds(1));

      if (!!m_clients[0].game_launch_event)
        break;
    }

  ASSERT_TRUE(!!m_clients[0].game_launch_event);

  // Start another game with only the second player. Since the first player is
  // in a game, a bot should be provided by the server.
  m_clients[1].new_game_auto_accept();

  // Let the time pass such that the messages can move between the clients and
  // the server.
  for (int i = 0; i != 10; ++i)
    {
      m_scheduler.tick(std::chrono::seconds(1));

      if (!!m_clients[1].game_launch_event)
        break;
    }

  ASSERT_TRUE(!!m_clients[1].game_launch_event);

  // The players should be in different games.
  EXPECT_NE(m_clients[0].game_launch_event->channel,
            m_clients[1].game_launch_event->channel);
}

TEST_F(server_bots_test, human_replaces_the_bot)
{
  m_clients[0].authenticate();
  m_clients[1].authenticate();

  // Start a game with only the first player. A bot should be provided by the
  // server.
  m_clients[0].new_game();

  // Let the time pass such that the messages can move between the clients and
  // the server.
  for (int i = 0; i != 10; ++i)
    {
      m_scheduler.tick(std::chrono::seconds(1));

      if (!!m_clients[0].player_count_proposal)
        break;
    }

  ASSERT_TRUE(!!m_clients[0].player_count_proposal);

  // Start another game with only the second player. Since the first player has
  // not accept the proposed game, both players should be matched.
  m_clients[1].new_game();

  // Let the time pass such that the messages can move between the clients and
  // the server.
  for (int i = 0; i != 10; ++i)
    {
      m_scheduler.tick(std::chrono::seconds(1));

      if (!!m_clients[1].player_count_proposal)
        break;
    }

  ASSERT_TRUE(!!m_clients[1].player_count_proposal);

  m_clients[0].accept_game();
  m_clients[1].accept_game();

  for (int i = 0; i != 10; ++i)
    {
      m_scheduler.tick(std::chrono::seconds(1));

      if (!!m_clients[0].game_launch_event && !!m_clients[1].game_launch_event)
        break;
    }

  ASSERT_TRUE(!!m_clients[0].game_launch_event);
  ASSERT_TRUE(!!m_clients[1].game_launch_event);

  // The players should be in the same game.
  EXPECT_EQ(m_clients[0].game_launch_event->channel,
            m_clients[1].game_launch_event->channel);
}
