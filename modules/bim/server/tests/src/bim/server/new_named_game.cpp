// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/fake_scheduler.hpp>

#include <bim/server/config.hpp>
#include <bim/server/server.hpp>

#include <bim/net/exchange/authentication_exchange.hpp>
#include <bim/net/message/accept_named_game.hpp>
#include <bim/net/message/game_on_hold.hpp>
#include <bim/net/message/launch_game.hpp>
#include <bim/net/message/new_named_game_request.hpp>
#include <bim/net/message/try_deserialize_message.hpp>

#include <iscool/log/setup.hpp>
#include <iscool/net/message_channel.hpp>
#include <iscool/signals/scoped_connection.hpp>

#include <optional>

#include <gtest/gtest.h>

class new_game_test : public testing::Test
{
protected:
  class client
  {
  public:
    client(bim::server::tests::fake_scheduler& scheduler,
           iscool::net::message_stream& message_stream);

    void authenticate();
    void send_new_named_game_request(
        const bim::net::new_named_game_request& message);
    void send_accept_named_game(const bim::net::accept_named_game& message);

  public:
    std::optional<bim::net::game_on_hold> game_on_hold_answer;
    std::optional<bim::net::launch_game> launch_game_answer;

  private:
    bim::server::tests::fake_scheduler& m_scheduler;

    bim::net::authentication_exchange m_authentication;
    std::unique_ptr<iscool::net::message_channel> m_message_channel;
  };

public:
  new_game_test();

  bim::net::client_token new_client_token();

protected:
  iscool::log::scoped_initializer m_log;
  bim::server::tests::fake_scheduler m_scheduler;

  const unsigned short m_port;
  bim::server::server m_server;
  iscool::net::socket_stream m_socket_stream;
  iscool::net::message_stream m_message_stream;

  std::array<client, 8> m_clients;

private:
  bim::net::client_token m_next_token;
};

new_game_test::client::client(bim::server::tests::fake_scheduler& scheduler,
                              iscool::net::message_stream& message_stream)
  : m_scheduler(scheduler)
  , m_authentication(message_stream)
{
  m_authentication.connect_to_authenticated(
      [this, &message_stream](iscool::net::session_id session) -> void
      {
        m_message_channel.reset(
            new iscool::net::message_channel(message_stream, session, 0));
      });

  m_authentication.connect_to_error(
      [this](bim::net::authentication_error_code) -> void
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
void new_game_test::client::send_new_named_game_request(
    const bim::net::new_named_game_request& message)
{
  game_on_hold_answer = std::nullopt;

  const bim::net::client_token token = message.get_request_token();

  const iscool::signals::scoped_connection connection =
      m_message_channel->connect_to_message(
          [this, token](const iscool::net::endpoint&,
                        const iscool::net::message& message) -> void
          {
            if (message.get_type() != bim::net::message_type::game_on_hold)
              return;

            std::optional<bim::net::game_on_hold> answer =
                bim::net::try_deserialize_message<bim::net::game_on_hold>(
                    message);

            if (answer->get_request_token() != token)
              return;

            game_on_hold_answer = std::move(*answer);
          });

  ASSERT_NE(nullptr, m_message_channel);

  for (int i = 0; (i != 10) && !game_on_hold_answer; ++i)
    {
      m_message_channel->send(message.build_message());
      m_scheduler.tick(std::chrono::milliseconds(50));
    }
}

void new_game_test::client::send_accept_named_game(
    const bim::net::accept_named_game& message)
{
  launch_game_answer = std::nullopt;

  const bim::net::client_token token = message.get_request_token();

  const iscool::signals::scoped_connection connection =
      m_message_channel->connect_to_message(
          [this, token](const iscool::net::endpoint&,
                        const iscool::net::message& message) -> void
          {
            if (message.get_type() != bim::net::message_type::launch_game)
              return;

            std::optional<bim::net::launch_game> answer =
                bim::net::try_deserialize_message<bim::net::launch_game>(
                    message);

            if (answer->get_request_token() != token)
              return;

            launch_game_answer = std::move(*answer);
          });

  ASSERT_NE(nullptr, m_message_channel);

  for (int i = 0; (i != 10) && !launch_game_answer; ++i)
    {
      m_message_channel->send(message.build_message());
      m_scheduler.tick(std::chrono::milliseconds(50));
    }
}

new_game_test::new_game_test()
  : m_port(10002)
  , m_server(bim::server::config{ .port = m_port })
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

bim::net::client_token new_game_test::new_client_token()
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

  const bim::net::client_token token = new_client_token();
  constexpr std::uint32_t feature_mask = 0;
  m_clients[0].send_new_named_game_request(bim::net::new_named_game_request(
      token, feature_mask, { 'a', 'b', 'c' }));

  ASSERT_TRUE(!!m_clients[0].game_on_hold_answer);

  EXPECT_EQ(token, m_clients[0].game_on_hold_answer->get_request_token());
  EXPECT_EQ(1, m_clients[0].game_on_hold_answer->get_player_count());
}

/** Many new game requests add to the player count. */
TEST_F(new_game_test, join_game)
{
  for (int i = 0; i != 4; ++i)
    m_clients[i].authenticate();

  bim::net::client_token tokens[] = { new_client_token(), new_client_token(),
                                      new_client_token(), new_client_token() };
  const bim::net::game_name game_name = { 'j', 'o', 'i', 'n' };
  constexpr std::uint32_t feature_mask = 0;

  // First player connects.
  m_clients[0].send_new_named_game_request(
      bim::net::new_named_game_request(tokens[0], feature_mask, game_name));

  ASSERT_TRUE(!!m_clients[0].game_on_hold_answer);

  EXPECT_EQ(tokens[0], m_clients[0].game_on_hold_answer->get_request_token());
  EXPECT_EQ(1, m_clients[0].game_on_hold_answer->get_player_count());

  // Second player connects.
  m_clients[1].send_new_named_game_request(
      bim::net::new_named_game_request(tokens[1], feature_mask, game_name));

  ASSERT_TRUE(!!m_clients[1].game_on_hold_answer);

  EXPECT_EQ(tokens[1], m_clients[1].game_on_hold_answer->get_request_token());

  // There are now two connected players.
  EXPECT_EQ(2, m_clients[1].game_on_hold_answer->get_player_count());

  // First player keep asking for a game
  m_clients[0].send_new_named_game_request(
      bim::net::new_named_game_request(tokens[0], feature_mask, game_name));

  ASSERT_TRUE(!!m_clients[0].game_on_hold_answer);
  EXPECT_EQ(tokens[0], m_clients[0].game_on_hold_answer->get_request_token());

  // First player should now see two connected players.
  EXPECT_EQ(2, m_clients[0].game_on_hold_answer->get_player_count());

  // Third player connects.
  m_clients[2].send_new_named_game_request(
      bim::net::new_named_game_request(tokens[2], feature_mask, game_name));

  ASSERT_TRUE(!!m_clients[2].game_on_hold_answer);

  EXPECT_EQ(tokens[2], m_clients[2].game_on_hold_answer->get_request_token());
  EXPECT_EQ(3, m_clients[2].game_on_hold_answer->get_player_count());

  // Fourth player connects.
  m_clients[3].send_new_named_game_request(
      bim::net::new_named_game_request(tokens[3], feature_mask, game_name));

  ASSERT_TRUE(!!m_clients[3].game_on_hold_answer);

  EXPECT_EQ(tokens[3], m_clients[3].game_on_hold_answer->get_request_token());
  EXPECT_EQ(4, m_clients[3].game_on_hold_answer->get_player_count());

  // New tokens for the next requests.
  for (int i = 0; i != 4; ++i)
    tokens[i] = new_client_token();

  // Refresh the request for all players.
  for (int i = 0; i != 4; ++i)
    m_clients[i].send_new_named_game_request(
        bim::net::new_named_game_request(tokens[i], feature_mask, game_name));

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

  // The encounter id should be the same for everyone.
  EXPECT_EQ(m_clients[0].game_on_hold_answer->get_encounter_id(),
            m_clients[1].game_on_hold_answer->get_encounter_id());
  EXPECT_EQ(m_clients[0].game_on_hold_answer->get_encounter_id(),
            m_clients[2].game_on_hold_answer->get_encounter_id());
  EXPECT_EQ(m_clients[0].game_on_hold_answer->get_encounter_id(),
            m_clients[3].game_on_hold_answer->get_encounter_id());
}

/** A non-responding player should be removed from the game. */
TEST_F(new_game_test, player_leaving_on_new_game)
{
  for (int i = 0; i != 4; ++i)
    m_clients[i].authenticate();

  const bim::net::game_name game_name = { 'k', 'i', 'c', 'k' };
  constexpr std::uint32_t feature_mask = 0;

  // Do the game request twice such that the first players know about the last
  // ones.
  for (int j = 0; j != 2; ++j)
    for (int i = 0; i != 4; ++i)
      m_clients[i].send_new_named_game_request(
          bim::net::new_named_game_request(new_client_token(), feature_mask,
                                           game_name));

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
      m_scheduler.tick(std::chrono::seconds(2));

      // Update everyone but one player.
      for (int i = 0; i != 4; ++i)
        if (i != inactive_index)
          m_clients[i].send_new_named_game_request(
              bim::net::new_named_game_request(new_client_token(),
                                               feature_mask, game_name));
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

  // The encounter id should be the same for the remaining players.
  for (int i = 0; i != 4; ++i)
    if (i != inactive_index)
      {
        EXPECT_EQ(m_clients[0].game_on_hold_answer->get_encounter_id(),
                  m_clients[i].game_on_hold_answer->get_encounter_id())
            << "i=" << i;
      }
}

/** When all players accept the game, the server sends a launch_game. */
TEST_F(new_game_test, accept_named_game)
{
  for (int i = 0; i != 4; ++i)
    m_clients[i].authenticate();

  const bim::net::game_name game_name = { 'a', 'c', 'c', 'e', 'p', 't' };
  constexpr std::uint32_t feature_mask = 0;

  for (int i = 0; i != 4; ++i)
    m_clients[i].send_new_named_game_request(bim::net::new_named_game_request(
        new_client_token(), feature_mask, game_name));

  // One iteration for each client to notify the server.
  for (int i = 0; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].game_on_hold_answer) << "i=" << i;
      m_clients[i].send_accept_named_game(bim::net::accept_named_game(
          new_client_token(),
          m_clients[i].game_on_hold_answer->get_encounter_id()));
    }

  // One iteration for each client get the launch_game.
  for (int i = 0; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].game_on_hold_answer) << "i=" << i;
      m_clients[i].send_accept_named_game(bim::net::accept_named_game(
          new_client_token(),
          m_clients[i].game_on_hold_answer->get_encounter_id()));
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
TEST_F(new_game_test, player_leaving_on_accept_named_game)
{
  for (int i = 0; i != 4; ++i)
    m_clients[i].authenticate();

  const bim::net::game_name game_name = { 'k', 'i', 'c', 'k' };
  constexpr std::uint32_t feature_mask = 0;

  // Do the game request twice such that the first players know about the last
  // ones.
  for (int j = 0; j != 2; ++j)
    for (int i = 0; i != 4; ++i)
      m_clients[i].send_new_named_game_request(
          bim::net::new_named_game_request(new_client_token(), feature_mask,
                                           game_name));

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
      m_scheduler.tick(std::chrono::seconds(2));

      // Update everyone but one player.
      for (int i = 0; i != 4; ++i)
        if (i != inactive_index)
          m_clients[i].send_accept_named_game(bim::net::accept_named_game(
              new_client_token(),
              m_clients[i].game_on_hold_answer->get_encounter_id()));
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

  const bim::net::game_name game_name = { 's', 'i', 'n', 'g', 'l', 'e' };
  constexpr std::uint32_t feature_mask = 0;

  m_clients[0].send_new_named_game_request(bim::net::new_named_game_request(
      new_client_token(), feature_mask, game_name));

  ASSERT_TRUE(!!m_clients[0].game_on_hold_answer);
  EXPECT_EQ(1, m_clients[0].game_on_hold_answer->get_player_count());

  m_clients[0].send_accept_named_game(bim::net::accept_named_game(
      new_client_token(),
      m_clients[0].game_on_hold_answer->get_encounter_id()));

  // Now we should not have a launch_game for our player.
  ASSERT_FALSE(!!m_clients[0].launch_game_answer);
}

/** Second player comes after the first one has accepted the game. There should
    be a launch_game message. */
TEST_F(new_game_test, late_second_player)
{
  const bim::net::game_name game_name = { 'l', 'a', 't', 'e' };
  constexpr std::uint32_t feature_mask = 0;

  for (int i = 0; i != 2; ++i)
    {
      m_clients[i].authenticate();

      m_clients[i].send_new_named_game_request(
          bim::net::new_named_game_request(new_client_token(), feature_mask,
                                           game_name));

      m_clients[i].send_accept_named_game(bim::net::accept_named_game(
          new_client_token(),
          m_clients[i].game_on_hold_answer->get_encounter_id()));
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
  m_clients[0].send_accept_named_game(bim::net::accept_named_game(
      new_client_token(),
      m_clients[0].game_on_hold_answer->get_encounter_id()));

  // Now the first player should have received a launch_game.
  ASSERT_TRUE(!!m_clients[0].launch_game_answer);

  EXPECT_EQ(m_clients[0].launch_game_answer->get_game_channel(),
            m_clients[1].launch_game_answer->get_game_channel());
}

/** The game upon creation should be destroyed if the players are inactive for
    too long. */
TEST_F(new_game_test, idle_causes_new_game)
{
  const bim::net::game_name game_name = { 'i', 'd', 'l', 'e' };
  constexpr std::uint32_t feature_mask = 0;

  for (int i = 0; i != 4; ++i)
    m_clients[i].authenticate();

  for (int i = 0; i != 4; ++i)
    m_clients[i].send_new_named_game_request(bim::net::new_named_game_request(
        new_client_token(), feature_mask, game_name));

  // All the players are grouped in the same game by the server.
  ASSERT_TRUE(!!m_clients[0].game_on_hold_answer);
  const bim::net::encounter_id encounter_id =
      m_clients[0].game_on_hold_answer->get_encounter_id();

  for (int i = 1; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].game_on_hold_answer);
      EXPECT_EQ(encounter_id,
                m_clients[i].game_on_hold_answer->get_encounter_id());
    }

  // Do nothing, such that the clean-up is triggered on the server.
  for (int i = 0; i != 5; ++i)
    m_scheduler.tick(std::chrono::minutes(1));

  // Try to join the same game.
  for (int i = 0; i != 4; ++i)
    m_clients[i].send_new_named_game_request(bim::net::new_named_game_request(
        new_client_token(), feature_mask, game_name));

  // The encounter id should have changed.
  for (int i = 0; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].game_on_hold_answer);
      EXPECT_NE(encounter_id,
                m_clients[i].game_on_hold_answer->get_encounter_id());
    }
}

/** No more than four players in a game. */
TEST_F(new_game_test, max_players_in_game)
{
  for (int i = 0; i != 8; ++i)
    m_clients[i].authenticate();

  const bim::net::game_name game_name = { 's', 'm', 'a', 'l', 'l' };
  constexpr std::uint32_t feature_mask = 0;

  // First batch of players.
  for (int i = 0; i != 4; ++i)
    m_clients[i].send_new_named_game_request(bim::net::new_named_game_request(
        new_client_token(), feature_mask, game_name));

  // Twice for each player to receive the player count.
  for (int i = 0; i != 4; ++i)
    m_clients[i].send_new_named_game_request(bim::net::new_named_game_request(
        new_client_token(), feature_mask, game_name));

  // Second batch of players.
  for (int i = 4; i != 8; ++i)
    m_clients[i].send_new_named_game_request(bim::net::new_named_game_request(
        new_client_token(), feature_mask, game_name));

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

/** Different games should produce different game channels. */
TEST_F(new_game_test, different_game_different_channels)
{
  for (int i = 0; i != 4; ++i)
    m_clients[i].authenticate();

  const bim::net::game_name game_names[2] = {
    { 'g', 'a', 'm', 'e', '_', '0' }, { 'g', 'a', 'm', 'e', '_', '1' }
  };
  constexpr std::uint32_t feature_mask = 0;

  for (int i = 0; i != 4; ++i)
    m_clients[i].send_new_named_game_request(bim::net::new_named_game_request(
        new_client_token(), feature_mask, game_names[i % 2]));

  // One iteration for each client to notify the server.
  for (int i = 0; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].game_on_hold_answer) << "i=" << i;
      m_clients[i].send_accept_named_game(bim::net::accept_named_game(
          new_client_token(),
          m_clients[i].game_on_hold_answer->get_encounter_id()));
    }

  // One iteration for each client get the launch_game.
  for (int i = 0; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].game_on_hold_answer) << "i=" << i;
      m_clients[i].send_accept_named_game(bim::net::accept_named_game(
          new_client_token(),
          m_clients[i].game_on_hold_answer->get_encounter_id()));
    }

  // Now we should have a launch_game for each pair of players.
  for (int i = 0; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].launch_game_answer) << "i=" << i;
      EXPECT_EQ(2, m_clients[i].launch_game_answer->get_player_count())
          << "i=" << i;
    }

  EXPECT_EQ(m_clients[0].launch_game_answer->get_game_channel(),
            m_clients[2].launch_game_answer->get_game_channel());
  EXPECT_EQ(m_clients[1].launch_game_answer->get_game_channel(),
            m_clients[3].launch_game_answer->get_game_channel());
  EXPECT_NE(m_clients[0].launch_game_answer->get_game_channel(),
            m_clients[1].launch_game_answer->get_game_channel());
}
