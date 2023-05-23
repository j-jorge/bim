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
#include <bm/server/server.hpp>

#include <bm/net/exchange/authentication_exchange.hpp>
#include <bm/net/message/accept_game.hpp>
#include <bm/net/message/game_on_hold.hpp>
#include <bm/net/message/launch_game.hpp>
#include <bm/net/message/new_game_request.hpp>

#include <iscool/log/setup.h>
#include <iscool/net/message_channel.h>
#include <iscool/net/message_deserializer.h>
#include <iscool/net/message_deserializer.impl.tpp>
#include <iscool/schedule/manual_scheduler.h>
#include <iscool/schedule/setup.h>
#include <iscool/signals/scoped_connection.h>
#include <iscool/time/setup.h>

#include <optional>

#include <gtest/gtest.h>

class new_game_test : public testing::Test
{
protected:
  class fake_scheduler
  {
  public:
    fake_scheduler();

    void tick(std::chrono::milliseconds duration);

  private:
    std::chrono::nanoseconds m_current_date;
    iscool::time::scoped_time_source_delegate m_time_source_initializer;
    iscool::schedule::manual_scheduler m_scheduler;
    iscool::schedule::scoped_scheduler_delegate m_scheduler_initializer;
  };

  class client
  {
  public:
    client(fake_scheduler& scheduler,
           iscool::net::message_stream& message_stream);

    void authenticate();
    void send_new_game_request(const bm::net::new_game_request& message);
    void send_accept_game(const bm::net::accept_game& message);

  public:
    std::optional<bm::net::game_on_hold> game_on_hold_answer;
    std::optional<bm::net::launch_game> launch_game_answer;

  private:
    fake_scheduler& m_scheduler;

    bm::net::authentication_exchange m_authentication;
    std::unique_ptr<iscool::net::message_channel> m_message_channel;
    iscool::net::message_deserializer m_message_deserializer;
  };

public:
  new_game_test();

  bm::net::client_token new_client_token();

protected:
  iscool::log::scoped_initializer m_log;
  fake_scheduler m_scheduler;

  const unsigned short m_port;
  bm::server::server m_server;
  iscool::net::socket_stream m_socket_stream;
  iscool::net::message_stream m_message_stream;

  std::array<client, 8> m_clients;

private:
  bm::net::client_token m_next_token;
};

new_game_test::fake_scheduler::fake_scheduler()
  : m_current_date{}
  , m_time_source_initializer(
        [this]() -> std::chrono::nanoseconds
        {
          return m_current_date;
        })
  , m_scheduler_initializer(m_scheduler.get_delayed_call_delegate())
{}

void new_game_test::fake_scheduler::tick(std::chrono::milliseconds duration)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  m_current_date += duration;
  m_scheduler.update_interval(duration);
}

new_game_test::client::client(fake_scheduler& scheduler,
                              iscool::net::message_stream& message_stream)
  : m_scheduler(scheduler)
  , m_authentication(message_stream)
{
  m_authentication.connect_to_authenticated(
      [this, &message_stream](iscool::net::session_id session) -> void
      {
        m_message_channel.reset(
            new iscool::net::message_channel(message_stream, session, 0));

        m_message_channel->connect_to_message(std::bind(
            &iscool::net::message_deserializer::interpret_received_message,
            &m_message_deserializer, std::placeholders::_1,
            std::placeholders::_2));
      });

  m_authentication.connect_to_error(
      [this](bm::net::authentication_error_code) -> void
      {
        EXPECT_TRUE(false);
      });
}

void new_game_test::client::authenticate()
{
  EXPECT_EQ(nullptr, m_message_channel);

  m_authentication.start();

  for (int i = 0; (i != 10) && !m_message_channel; ++i)
    m_scheduler.tick(std::chrono::seconds(1));
}

/// Send a new game request message until we get an answer.
void new_game_test::client::send_new_game_request(
    const bm::net::new_game_request& message)
{
  game_on_hold_answer = std::nullopt;

  const bm::net::client_token token = message.get_request_token();

  const iscool::signals::scoped_connection connection
      = m_message_deserializer.connect_signal<bm::net::game_on_hold>(
          [this, token](const iscool::net::endpoint&,
                        bm::net::game_on_hold answer) -> void
          {
            if (answer.get_request_token() != token)
              return;

            game_on_hold_answer = std::move(answer);
          });

  ASSERT_NE(nullptr, m_message_channel);

  for (int i = 0; (i != 10) && !game_on_hold_answer; ++i)
    {
      m_message_channel->send(message.build_message());
      m_scheduler.tick(std::chrono::seconds(1));
    }
}

void new_game_test::client::send_accept_game(
    const bm::net::accept_game& message)
{
  launch_game_answer = std::nullopt;

  const bm::net::client_token token = message.get_request_token();

  const iscool::signals::scoped_connection connection
      = m_message_deserializer.connect_signal<bm::net::launch_game>(
          [this, token](const iscool::net::endpoint&,
                        bm::net::launch_game answer) -> void
          {
            if (answer.get_request_token() != token)
              return;

            launch_game_answer = std::move(answer);
          });

  ASSERT_NE(nullptr, m_message_channel);

  for (int i = 0; (i != 10) && !launch_game_answer; ++i)
    {
      m_message_channel->send(message.build_message());
      m_scheduler.tick(std::chrono::seconds(1));
    }
}

new_game_test::new_game_test()
  : m_port(10002)
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
               client(m_scheduler, m_message_stream),
               client(m_scheduler, m_message_stream) }
  , m_next_token(0)
{}

bm::net::client_token new_game_test::new_client_token()
{
  return m_next_token++;
}

/**
 * A request for a new game from an authenticated player results in a
 * game_on_hold message with one player.
 */
TEST_F(new_game_test, answer_is_game_on_hold)
{
  m_clients[0].authenticate();

  const bm::net::client_token token = new_client_token();
  m_clients[0].send_new_game_request(
      bm::net::new_game_request(token, { 'a', 'b', 'c' }));

  ASSERT_TRUE(!!m_clients[0].game_on_hold_answer);

  EXPECT_EQ(token, m_clients[0].game_on_hold_answer->get_request_token());
  EXPECT_EQ(1, m_clients[0].game_on_hold_answer->get_player_count());
}

/** Many new game requests add to the player count. */
TEST_F(new_game_test, join_game)
{
  for (int i = 0; i != 4; ++i)
    m_clients[i].authenticate();

  bm::net::client_token tokens[] = { new_client_token(), new_client_token(),
                                     new_client_token(), new_client_token() };
  const bm::net::game_name game_name = { 'j', 'o', 'i', 'n' };

  // First player connects.
  m_clients[0].send_new_game_request(
      bm::net::new_game_request(tokens[0], game_name));

  ASSERT_TRUE(!!m_clients[0].game_on_hold_answer);

  EXPECT_EQ(tokens[0], m_clients[0].game_on_hold_answer->get_request_token());
  EXPECT_EQ(1, m_clients[0].game_on_hold_answer->get_player_count());

  // Second player connects.
  m_clients[1].send_new_game_request(
      bm::net::new_game_request(tokens[1], game_name));

  ASSERT_TRUE(!!m_clients[1].game_on_hold_answer);

  EXPECT_EQ(tokens[1], m_clients[1].game_on_hold_answer->get_request_token());

  // There are now two connected players.
  EXPECT_EQ(2, m_clients[1].game_on_hold_answer->get_player_count());

  // First player keep asking for a game
  m_clients[0].send_new_game_request(
      bm::net::new_game_request(tokens[0], game_name));

  ASSERT_TRUE(!!m_clients[0].game_on_hold_answer);
  EXPECT_EQ(tokens[0], m_clients[0].game_on_hold_answer->get_request_token());

  // First player should now see two connected players.
  EXPECT_EQ(2, m_clients[0].game_on_hold_answer->get_player_count());

  // Third player connects.
  m_clients[2].send_new_game_request(
      bm::net::new_game_request(tokens[2], game_name));

  ASSERT_TRUE(!!m_clients[2].game_on_hold_answer);

  EXPECT_EQ(tokens[2], m_clients[2].game_on_hold_answer->get_request_token());
  EXPECT_EQ(3, m_clients[2].game_on_hold_answer->get_player_count());

  // Fourth player connects.
  m_clients[3].send_new_game_request(
      bm::net::new_game_request(tokens[3], game_name));

  ASSERT_TRUE(!!m_clients[3].game_on_hold_answer);

  EXPECT_EQ(tokens[3], m_clients[3].game_on_hold_answer->get_request_token());
  EXPECT_EQ(4, m_clients[3].game_on_hold_answer->get_player_count());

  // New tokens for the next requests.
  for (int i = 0; i != 4; ++i)
    tokens[i] = new_client_token();

  // Refresh the request for all players.
  for (int i = 0; i != 4; ++i)
    m_clients[i].send_new_game_request(
        bm::net::new_game_request(tokens[i], game_name));

  // Every player should see four players.
  for (int i = 0; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].game_on_hold_answer) << "i=" << i;

      EXPECT_EQ(tokens[i],
                m_clients[i].game_on_hold_answer->get_request_token())
          << "i=" << i;
      EXPECT_EQ(4, m_clients[i].game_on_hold_answer->get_player_count())
          << "i=" << i;
    }

  // The game id should be the same for everyone.
  EXPECT_EQ(m_clients[0].game_on_hold_answer->get_game_id(),
            m_clients[1].game_on_hold_answer->get_game_id());
  EXPECT_EQ(m_clients[0].game_on_hold_answer->get_game_id(),
            m_clients[2].game_on_hold_answer->get_game_id());
  EXPECT_EQ(m_clients[0].game_on_hold_answer->get_game_id(),
            m_clients[3].game_on_hold_answer->get_game_id());
}

/** A non-responding player should be removed from the game. */
TEST_F(new_game_test, player_leaving_on_new_game)
{
  for (int i = 0; i != 4; ++i)
    m_clients[i].authenticate();

  const bm::net::game_name game_name = { 'k', 'i', 'c', 'k' };

  // Do the game request twice such that the first players know about the last
  // ones.
  for (int j = 0; j != 2; ++j)
    for (int i = 0; i != 4; ++i)
      m_clients[i].send_new_game_request(
          bm::net::new_game_request(new_client_token(), game_name));

  // All clients are connected and should now see four players.
  for (int i = 0; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].game_on_hold_answer) << "i=" << i;
      EXPECT_EQ(4, m_clients[i].game_on_hold_answer->get_player_count())
          << "i=" << i;
    }

  const int inactive_index = 2;

  // Let the time pass to trigger the kicking of the inactive players.
  for (int t = 0; t != 3; ++t)
    {
      m_scheduler.tick(std::chrono::seconds(30));

      // Update everyone but one player.
      for (int i = 0; i != 4; ++i)
        if (i != inactive_index)
          m_clients[i].send_new_game_request(
              bm::net::new_game_request(new_client_token(), game_name));
    }

  // All the players that have sent updates should see three players.
  for (int i = 0; i != 4; ++i)
    if (i != inactive_index)
      {
        ASSERT_TRUE(!!m_clients[i].game_on_hold_answer) << "i=" << i;
        EXPECT_EQ(3, m_clients[i].game_on_hold_answer->get_player_count())
            << "i=" << i;
      }

  ASSERT_NE(0, inactive_index);

  // The game id should be the same for the remaining players.
  for (int i = 0; i != 4; ++i)
    if (i != inactive_index)
      {
        EXPECT_EQ(m_clients[0].game_on_hold_answer->get_game_id(),
                  m_clients[i].game_on_hold_answer->get_game_id())
            << "i=" << i;
      }
}

/** When all players accept the game, the server sends a launch_game. */
TEST_F(new_game_test, accept_game)
{
  for (int i = 0; i != 4; ++i)
    m_clients[i].authenticate();

  const bm::net::game_name game_name = { 'a', 'c', 'c', 'e', 'p', 't' };

  for (int i = 0; i != 4; ++i)
    m_clients[i].send_new_game_request(
        bm::net::new_game_request(new_client_token(), game_name));

  // One iteration for each client to notify the server.
  for (int i = 0; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].game_on_hold_answer) << "i=" << i;
      m_clients[i].send_accept_game(bm::net::accept_game(
          new_client_token(),
          m_clients[i].game_on_hold_answer->get_game_id()));
    }

  // One iteration for each client get the launch_game.
  for (int i = 0; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].game_on_hold_answer) << "i=" << i;
      m_clients[i].send_accept_game(bm::net::accept_game(
          new_client_token(),
          m_clients[i].game_on_hold_answer->get_game_id()));
    }

  // Now we should have a launch_game for four players.
  for (int i = 0; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].launch_game_answer) << "i=" << i;
      EXPECT_EQ(4, m_clients[i].launch_game_answer->get_player_count())
          << "i=" << i;
    }

  EXPECT_EQ(m_clients[0].launch_game_answer->get_game_channel(),
            m_clients[1].launch_game_answer->get_game_channel());
  EXPECT_EQ(m_clients[0].launch_game_answer->get_game_channel(),
            m_clients[2].launch_game_answer->get_game_channel());
  EXPECT_EQ(m_clients[0].launch_game_answer->get_game_channel(),
            m_clients[3].launch_game_answer->get_game_channel());
}

/** A non-responding player should be removed from the game. */
TEST_F(new_game_test, player_leaving_on_accept_game)
{
  for (int i = 0; i != 4; ++i)
    m_clients[i].authenticate();

  const bm::net::game_name game_name = { 'k', 'i', 'c', 'k' };

  // Do the game request twice such that the first players know about the last
  // ones.
  for (int j = 0; j != 2; ++j)
    for (int i = 0; i != 4; ++i)
      m_clients[i].send_new_game_request(
          bm::net::new_game_request(new_client_token(), game_name));

  // All clients are connected and should now see four players.
  for (int i = 0; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].game_on_hold_answer) << "i=" << i;
      EXPECT_EQ(4, m_clients[i].game_on_hold_answer->get_player_count())
          << "i=" << i;
    }

  const int inactive_index = 2;

  // Let the time pass to trigger the kicking of the inactive players.
  for (int t = 0; t != 3; ++t)
    {
      m_scheduler.tick(std::chrono::seconds(30));

      // Update everyone but one player.
      for (int i = 0; i != 4; ++i)
        if (i != inactive_index)
          m_clients[i].send_accept_game(bm::net::accept_game(
              new_client_token(),
              m_clients[i].game_on_hold_answer->get_game_id()));
    }

  // All the players that have sent updates should have received a launch_game
  // with a count of three players..
  for (int i = 0; i != 4; ++i)
    if (i != inactive_index)
      {
        ASSERT_TRUE(!!m_clients[i].launch_game_answer) << "i=" << i;
        EXPECT_EQ(3, m_clients[i].launch_game_answer->get_player_count())
            << "i=" << i;
      }

  ASSERT_NE(0, inactive_index);

  // The game channel should be the same for the remaining players.
  for (int i = 0; i != 4; ++i)
    if (i != inactive_index)
      {
        EXPECT_EQ(m_clients[0].launch_game_answer->get_game_channel(),
                  m_clients[i].launch_game_answer->get_game_channel())
            << "i=" << i;
      }
}

/** A player accepting a game with a player count of one should not trigger a
    launch_game message. */
TEST_F(new_game_test, no_single_player_game)
{
  m_clients[0].authenticate();

  const bm::net::game_name game_name = { 's', 'i', 'n', 'g', 'l', 'e' };

  m_clients[0].send_new_game_request(
      bm::net::new_game_request(new_client_token(), game_name));

  ASSERT_TRUE(!!m_clients[0].game_on_hold_answer);
  EXPECT_EQ(1, m_clients[0].game_on_hold_answer->get_player_count());

  m_clients[0].send_accept_game(bm::net::accept_game(
      new_client_token(), m_clients[0].game_on_hold_answer->get_game_id()));

  // Now we should not have a launch_game for our player.
  ASSERT_FALSE(!!m_clients[0].launch_game_answer);
}

/** Second player comes after the first one has accepted the game. There should
    be a launch_game message. */
TEST_F(new_game_test, late_second_player)
{
  const bm::net::game_name game_name = { 'l', 'a', 't', 'e' };

  for (int i = 0; i != 2; ++i)
    {
      m_clients[i].authenticate();

      m_clients[i].send_new_game_request(
          bm::net::new_game_request(new_client_token(), game_name));

      m_clients[i].send_accept_game(bm::net::accept_game(
          new_client_token(),
          m_clients[i].game_on_hold_answer->get_game_id()));
    }

  // The first player only knows about him.
  ASSERT_TRUE(!!m_clients[0].game_on_hold_answer);
  EXPECT_EQ(1, m_clients[0].game_on_hold_answer->get_player_count());

  // The second player knows about the first player.
  ASSERT_TRUE(!!m_clients[1].game_on_hold_answer);
  EXPECT_EQ(2, m_clients[1].game_on_hold_answer->get_player_count());

  // The acceptance order is first player then second player, thus only the
  // second player should have received the launch_game.
  ASSERT_FALSE(!!m_clients[0].launch_game_answer);
  ASSERT_TRUE(!!m_clients[1].launch_game_answer);

  // Accept again for the first player to trigger an answer from the server.
  ASSERT_TRUE(!!m_clients[0].game_on_hold_answer);
  m_clients[0].send_accept_game(bm::net::accept_game(
      new_client_token(), m_clients[0].game_on_hold_answer->get_game_id()));

  // Now the first player should have received a launch_game.
  ASSERT_TRUE(!!m_clients[0].launch_game_answer);

  EXPECT_EQ(m_clients[0].launch_game_answer->get_game_channel(),
            m_clients[1].launch_game_answer->get_game_channel());
}

/** The game upon creation should be destroyed if the players are inactive for
    too long. */
TEST_F(new_game_test, idle_causes_new_game)
{
  const bm::net::game_name game_name = { 'i', 'd', 'l', 'e' };

  for (int i = 0; i != 4; ++i)
    m_clients[i].authenticate();

  for (int i = 0; i != 4; ++i)
    m_clients[i].send_new_game_request(
        bm::net::new_game_request(new_client_token(), game_name));

  // All the players are grouped in the same game by the server.
  ASSERT_TRUE(!!m_clients[0].game_on_hold_answer);
  const bm::net::game_id game_id
      = m_clients[0].game_on_hold_answer->get_game_id();

  for (int i = 1; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].game_on_hold_answer);
      EXPECT_EQ(game_id, m_clients[i].game_on_hold_answer->get_game_id());
    }

  // Do nothing, such that the clean-up is triggered on the server.
  for (int i = 0; i != 5; ++i)
    m_scheduler.tick(std::chrono::minutes(1));

  // Try to join the same game.
  for (int i = 0; i != 4; ++i)
    m_clients[i].send_new_game_request(
        bm::net::new_game_request(new_client_token(), game_name));

  // The game ID should have changed.
  for (int i = 0; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].game_on_hold_answer);
      EXPECT_NE(game_id, m_clients[i].game_on_hold_answer->get_game_id());
    }
}

/** No more than four players in a game. */
TEST_F(new_game_test, max_players_in_game)
{
  for (int i = 0; i != 8; ++i)
    m_clients[i].authenticate();

  const bm::net::game_name game_name = { 's', 'm', 'a', 'l', 'l' };

  // First batch of players.
  for (int i = 0; i != 4; ++i)
    m_clients[i].send_new_game_request(
        bm::net::new_game_request(new_client_token(), game_name));

  // Twice for each player to receive the player count.
  for (int i = 0; i != 4; ++i)
    m_clients[i].send_new_game_request(
        bm::net::new_game_request(new_client_token(), game_name));

  // Second batch of players.
  for (int i = 4; i != 8; ++i)
    m_clients[i].send_new_game_request(
        bm::net::new_game_request(new_client_token(), game_name));

  // The clients from the first batch are in the game.
  for (int i = 0; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].game_on_hold_answer) << "i=" << i;
      EXPECT_EQ(4, m_clients[i].game_on_hold_answer->get_player_count())
          << "i=" << i;
    }

  // The clients from the second batch have no game.
  for (int i = 4; i != 8; ++i)
    EXPECT_FALSE(!!m_clients[i].game_on_hold_answer) << "i=" << i;
}
