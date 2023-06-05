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
#include <bm/server/tests/fake_scheduler.hpp>

#include <bm/server/server.hpp>

#include <bm/net/exchange/authentication_exchange.hpp>
#include <bm/net/exchange/game_update_exchange.hpp>
#include <bm/net/exchange/new_game_exchange.hpp>

#include <iscool/log/setup.h>
#include <iscool/net/message_channel.h>
#include <iscool/net/message_deserializer.h>
#include <iscool/net/message_deserializer.impl.tpp>
#include <iscool/signals/scoped_connection.h>

#include <iscool/log/enable_console_log.h>

#include <optional>

#include <gtest/gtest.h>

class game_update_test : public testing::Test
{
protected:
  class client
  {
  public:
    client(bm::server::tests::fake_scheduler& scheduler,
           iscool::net::message_stream& message_stream);

    void authenticate();
    void new_game(const bm::net::game_name& name);

  public:
    std::optional<iscool::net::channel_id> m_channel;
    std::optional<unsigned> m_player_count;
    std::optional<unsigned> m_player_index;
    std::optional<bool> m_started;

  private:
    void launch_game(iscool::net::message_stream& stream,
                     iscool::net::session_id session,
                     iscool::net::channel_id channel, unsigned player_count,
                     unsigned player_index);

  private:
    bm::server::tests::fake_scheduler& m_scheduler;

    bm::net::authentication_exchange m_authentication;
    std::unique_ptr<bm::net::new_game_exchange> m_new_game;
    std::unique_ptr<iscool::net::message_channel> m_message_channel;
    std::unique_ptr<bm::net::game_update_exchange> m_game_update;
  };

public:
  game_update_test();

protected:
  void wait();

protected:
  iscool::log::scoped_initializer m_log;
  bm::server::tests::fake_scheduler m_scheduler;

  const unsigned short m_port;
  bm::server::server m_server;
  iscool::net::socket_stream m_socket_stream;
  iscool::net::message_stream m_message_stream;

  std::array<client, 4> m_clients;
};

game_update_test::client::client(bm::server::tests::fake_scheduler& scheduler,
                                 iscool::net::message_stream& message_stream)
  : m_scheduler(scheduler)
  , m_authentication(message_stream)
{
  m_authentication.connect_to_authenticated(
      [this, &message_stream](iscool::net::session_id session) -> void
      {
        m_new_game.reset(
            new bm::net::new_game_exchange(message_stream, session));

        m_new_game->connect_to_game_proposal(
            std::bind(&bm::net::new_game_exchange::accept, m_new_game.get()));

        m_new_game->connect_to_launch_game(
            std::bind(&client::launch_game, this, std::ref(message_stream),
                      session, std::placeholders::_1, std::placeholders::_2,
                      std::placeholders::_3));
      });

  m_authentication.connect_to_error(
      [this](bm::net::authentication_error_code) -> void
      {
        EXPECT_TRUE(false);
      });
}

void game_update_test::client::authenticate()
{
  EXPECT_EQ(nullptr, m_new_game);

  m_authentication.start();

  for (int i = 0; (i != 10) && !m_new_game; ++i)
    m_scheduler.tick(std::chrono::seconds(1));
}

void game_update_test::client::new_game(const bm::net::game_name& name)
{
  ASSERT_NE(nullptr, m_new_game);

  m_new_game->start(name);
}

void game_update_test::client::launch_game(iscool::net::message_stream& stream,
                                           iscool::net::session_id session,
                                           iscool::net::channel_id channel,
                                           unsigned player_count,
                                           unsigned player_index)
{
  EXPECT_FALSE(!!m_channel);
  m_channel = channel;

  EXPECT_FALSE(!!m_player_count);
  m_player_count = player_count;

  EXPECT_FALSE(!!m_player_index);
  m_player_index = player_index;

  m_message_channel.reset(
      new iscool::net::message_channel(stream, session, channel));
  m_game_update.reset(
      new bm::net::game_update_exchange(*m_message_channel, player_count));

  m_game_update->connect_to_started(
      [this]() -> void
      {
        m_started.emplace(true);
      });

  m_game_update->start();
}

game_update_test::game_update_test()
  : m_port(10004)
  , m_server(m_port)
  , m_socket_stream("localhost:" + std::to_string(m_port),
                    iscool::net::socket_mode::client{})
  , m_message_stream(m_socket_stream)
  , m_clients{ client(m_scheduler, m_message_stream),
               client(m_scheduler, m_message_stream),
               client(m_scheduler, m_message_stream),
               client(m_scheduler, m_message_stream) }
{
  iscool::log::enable_console_log();
}

void game_update_test::wait()
{
  for (int i = 0; i != 100; ++i)
    {
      std::this_thread::sleep_for(std::chrono::seconds(0));
      m_scheduler.tick(std::chrono::seconds(1));
    }
}

/** The game should begin with a bm::net::start message. */
TEST_F(game_update_test, start)
{
  for (int i = 0; i != 4; ++i)
    m_clients[i].authenticate();

  for (int i = 0; i != 4; ++i)
    m_clients[i].new_game({ 'u', 'p', 'd', 'a', 't', 'e', '1' });

  // Let the time pass such that the messages can move between the clients and
  // the server.
  wait();

  for (int i = 0; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].m_started) << "i=" << i;
      EXPECT_TRUE(*m_clients[i].m_started) << "i=" << i;
    }
}
