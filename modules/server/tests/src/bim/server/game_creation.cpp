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
#include <bim/server/tests/fake_scheduler.hpp>

#include <bim/server/server.hpp>

#include <bim/net/exchange/authentication_exchange.hpp>
#include <bim/net/exchange/new_game_exchange.hpp>

#include <iscool/log/setup.h>
#include <iscool/net/message_channel.h>
#include <iscool/net/message_deserializer.h>
#include <iscool/net/message_deserializer.impl.tpp>
#include <iscool/signals/scoped_connection.h>

#include <optional>

#include <gtest/gtest.h>

class game_creation_test : public testing::Test
{
protected:
  class client
  {
  public:
    client(bim::server::tests::fake_scheduler& scheduler,
           iscool::net::message_stream& message_stream);

    void authenticate();
    void new_game(const bim::net::game_name& name);

  public:
    std::optional<iscool::net::channel_id> m_channel;
    std::optional<unsigned> m_player_count;
    std::optional<unsigned> m_player_index;

  private:
    void launch_game(iscool::net::channel_id channel, unsigned player_count,
                     unsigned player_index);

  private:
    bim::server::tests::fake_scheduler& m_scheduler;

    bim::net::authentication_exchange m_authentication;
    std::unique_ptr<bim::net::new_game_exchange> m_new_game;
  };

public:
  game_creation_test();

protected:
  iscool::log::scoped_initializer m_log;
  bim::server::tests::fake_scheduler m_scheduler;

  const unsigned short m_port;
  bim::server::server m_server;
  iscool::net::socket_stream m_socket_stream;
  iscool::net::message_stream m_message_stream;

  std::array<client, 7> m_clients;
};

game_creation_test::client::client(
    bim::server::tests::fake_scheduler& scheduler,
    iscool::net::message_stream& message_stream)
  : m_scheduler(scheduler)
  , m_authentication(message_stream)
{
  m_authentication.connect_to_authenticated(
      [this, &message_stream](iscool::net::session_id session) -> void
      {
        m_new_game.reset(
            new bim::net::new_game_exchange(message_stream, session));

        m_new_game->connect_to_game_proposal(
            std::bind(&bim::net::new_game_exchange::accept, m_new_game.get()));

        m_new_game->connect_to_launch_game(
            std::bind(&client::launch_game, this, std::placeholders::_1,
                      std::placeholders::_2, std::placeholders::_3));
      });

  m_authentication.connect_to_error(
      [this](bim::net::authentication_error_code) -> void
      {
        EXPECT_TRUE(false);
      });
}

void game_creation_test::client::authenticate()
{
  EXPECT_EQ(nullptr, m_new_game);

  m_authentication.start();

  for (int i = 0; (i != 10) && !m_new_game; ++i)
    m_scheduler.tick(std::chrono::seconds(1));
}

void game_creation_test::client::new_game(const bim::net::game_name& name)
{
  ASSERT_NE(nullptr, m_new_game);

  m_new_game->start(name);
}

void game_creation_test::client::launch_game(iscool::net::channel_id channel,
                                             unsigned player_count,
                                             unsigned player_index)
{
  EXPECT_FALSE(!!m_channel);
  m_channel = channel;

  EXPECT_FALSE(!!m_player_count);
  m_player_count = player_count;

  EXPECT_FALSE(!!m_player_index);
  m_player_index = player_index;
}

game_creation_test::game_creation_test()
  : m_port(10003)
  , m_server(m_port)
  , m_socket_stream("localhost:" + std::to_string(m_port),
                    iscool::net::socket_mode::client{})
  , m_message_stream(m_socket_stream)
  , m_clients{ client(m_scheduler, m_message_stream),
               client(m_scheduler, m_message_stream),
               client(m_scheduler, m_message_stream),
               client(m_scheduler, m_message_stream),
               client(m_scheduler, m_message_stream),
               client(m_scheduler, m_message_stream),
               client(m_scheduler, m_message_stream) }
{}

/** Ensure that the new_game_exchange creates and joins the game. */
TEST_F(game_creation_test, two_games)
{
  for (int i = 0; i != 7; ++i)
    m_clients[i].authenticate();

  for (int i = 0; i != 4; ++i)
    m_clients[i].new_game({ 'g', 'a', 'm', 'e', '1' });

  // Let the time pass such that the messages can move between the clients and
  // the server.
  for (int i = 0; i != 10; ++i)
    m_scheduler.tick(std::chrono::seconds(1));

  for (int i = 4; i != 7; ++i)
    m_clients[i].new_game({ 'g', 'a', 'm', 'e', '2' });

  // Time passes again…
  for (int i = 0; i != 10; ++i)
    m_scheduler.tick(std::chrono::seconds(1));

  for (int i = 0; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].m_channel) << "i=" << i;

      ASSERT_TRUE(!!m_clients[i].m_player_count) << "i=" << i;
      EXPECT_EQ(4, *m_clients[i].m_player_count) << "i=" << i;

      ASSERT_TRUE(!!m_clients[i].m_player_index) << "i=" << i;
    }

  // The player index must all be different.
  EXPECT_NE(*m_clients[0].m_player_index, *m_clients[1].m_player_index);
  EXPECT_NE(*m_clients[0].m_player_index, *m_clients[2].m_player_index);
  EXPECT_NE(*m_clients[0].m_player_index, *m_clients[3].m_player_index);
  EXPECT_NE(*m_clients[1].m_player_index, *m_clients[2].m_player_index);
  EXPECT_NE(*m_clients[1].m_player_index, *m_clients[3].m_player_index);
  EXPECT_NE(*m_clients[2].m_player_index, *m_clients[3].m_player_index);

  // The game channel must be the same
  EXPECT_EQ(*m_clients[0].m_channel, *m_clients[1].m_channel);
  EXPECT_EQ(*m_clients[0].m_channel, *m_clients[2].m_channel);
  EXPECT_EQ(*m_clients[0].m_channel, *m_clients[3].m_channel);

  for (int i = 4; i != 7; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].m_channel) << "i=" << i;

      ASSERT_TRUE(!!m_clients[i].m_player_count) << "i=" << i;
      EXPECT_EQ(3, *m_clients[i].m_player_count) << "i=" << i;

      ASSERT_TRUE(!!m_clients[i].m_player_index) << "i=" << i;
    }

  // The player index must all be different.
  EXPECT_NE(*m_clients[4].m_player_index, *m_clients[5].m_player_index);
  EXPECT_NE(*m_clients[4].m_player_index, *m_clients[6].m_player_index);
  EXPECT_NE(*m_clients[5].m_player_index, *m_clients[6].m_player_index);

  // The game channel must be the same
  EXPECT_EQ(*m_clients[4].m_channel, *m_clients[5].m_channel);
  EXPECT_EQ(*m_clients[4].m_channel, *m_clients[6].m_channel);

  // The game channel of each group of players must be different.
  EXPECT_NE(*m_clients[0].m_channel, *m_clients[4].m_channel);
}
