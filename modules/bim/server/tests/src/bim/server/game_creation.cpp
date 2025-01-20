// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/fake_scheduler.hpp>

#include <bim/server/config.hpp>
#include <bim/server/server.hpp>

#include <bim/net/exchange/authentication_exchange.hpp>
#include <bim/net/exchange/game_launch_event.hpp>
#include <bim/net/exchange/new_game_exchange.hpp>

#include <iscool/log/setup.hpp>
#include <iscool/net/message_channel.hpp>
#include <iscool/signals/scoped_connection.hpp>

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
    void new_game(const bim::net::game_name& name,
                  bim::game::feature_flags features);
    void new_game(bim::game::feature_flags features);

  public:
    std::optional<bim::net::game_launch_event> m_game_launch_event;
    std::optional<iscool::net::session_id> m_session;

  private:
    void launch_game(const bim::net::game_launch_event& event);

  private:
    bim::server::tests::fake_scheduler& m_scheduler;

    bim::net::authentication_exchange m_authentication;
    bim::net::new_game_exchange m_new_game;
  };

public:
  game_creation_test();

  void validate_player_order();

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
  , m_new_game(message_stream)
{
  m_authentication.connect_to_authenticated(
      [this, &message_stream](iscool::net::session_id session) -> void
      {
        m_session = session;
      });

  m_authentication.connect_to_error(
      [this](bim::net::authentication_error_code) -> void
      {
        EXPECT_TRUE(false);
      });

  m_new_game.connect_to_launch_game(
      std::bind(&client::launch_game, this, std::placeholders::_1));
}

void game_creation_test::client::authenticate()
{
  ASSERT_FALSE(!!m_session);

  m_authentication.start();

  for (int i = 0; (i != 10) && !m_session; ++i)
    m_scheduler.tick(std::chrono::seconds(1));
}

void game_creation_test::client::new_game(const bim::net::game_name& name,
                                          bim::game::feature_flags features)
{
  ASSERT_TRUE(!!m_session);

  m_new_game.connect_to_game_proposal(
      std::bind(&bim::net::new_game_exchange::accept, &m_new_game, features));

  m_new_game.start(*m_session, name);
}

void game_creation_test::client::new_game(bim::game::feature_flags features)
{
  ASSERT_TRUE(!!m_session);

  m_new_game.connect_to_game_proposal(
      std::bind(&bim::net::new_game_exchange::accept, &m_new_game, features));
  m_new_game.start(*m_session);
}

void game_creation_test::client::launch_game(
    const bim::net::game_launch_event& event)
{
  EXPECT_FALSE(!!m_game_launch_event);
  m_game_launch_event = event;
}

game_creation_test::game_creation_test()
  : m_port(10003)
  , m_server(bim::server::config(m_port))
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
TEST_F(game_creation_test, two_named_games)
{
  for (int i = 0; i != 7; ++i)
    m_clients[i].authenticate();

  for (int i = 0; i != 4; ++i)
    m_clients[i].new_game({ 'g', 'a', 'm', 'e', '1' },
                          bim::game::feature_flags(0b1110 | (1 << i)));

  // Let the time pass such that the messages can move between the clients and
  // the server.
  for (int i = 0; i != 10; ++i)
    m_scheduler.tick(std::chrono::seconds(1));

  for (int i = 4; i != 7; ++i)
    m_clients[i].new_game({ 'g', 'a', 'm', 'e', '2' },
                          bim::game::feature_flags(0b1110'0000 | (1 << i)));

  // Time passes again…
  for (int i = 0; i != 10; ++i)
    m_scheduler.tick(std::chrono::seconds(1));

  for (int i = 0; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].m_game_launch_event) << "i=" << i;
      EXPECT_EQ(4, m_clients[i].m_game_launch_event->player_count)
          << "i=" << i;
    }

  // The player index must all be different.
  EXPECT_NE(m_clients[0].m_game_launch_event->player_index,
            m_clients[1].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[0].m_game_launch_event->player_index,
            m_clients[2].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[0].m_game_launch_event->player_index,
            m_clients[3].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[1].m_game_launch_event->player_index,
            m_clients[2].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[1].m_game_launch_event->player_index,
            m_clients[3].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[2].m_game_launch_event->player_index,
            m_clients[3].m_game_launch_event->player_index);

  // The game channel must be the same.
  EXPECT_EQ(m_clients[0].m_game_launch_event->channel,
            m_clients[1].m_game_launch_event->channel);
  EXPECT_EQ(m_clients[0].m_game_launch_event->channel,
            m_clients[2].m_game_launch_event->channel);
  EXPECT_EQ(m_clients[0].m_game_launch_event->channel,
            m_clients[3].m_game_launch_event->channel);

  // The features must be the same.
  EXPECT_EQ(bim::game::feature_flags(0b1110),
            m_clients[0].m_game_launch_event->features);
  EXPECT_EQ(m_clients[0].m_game_launch_event->features,
            m_clients[1].m_game_launch_event->features);
  EXPECT_EQ(m_clients[0].m_game_launch_event->features,
            m_clients[2].m_game_launch_event->features);
  EXPECT_EQ(m_clients[0].m_game_launch_event->features,
            m_clients[3].m_game_launch_event->features);

  for (int i = 4; i != 7; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].m_game_launch_event) << "i=" << i;
      EXPECT_EQ(3, m_clients[i].m_game_launch_event->player_count)
          << "i=" << i;
    }

  // The player index must all be different.
  EXPECT_NE(m_clients[4].m_game_launch_event->player_index,
            m_clients[5].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[4].m_game_launch_event->player_index,
            m_clients[6].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[5].m_game_launch_event->player_index,
            m_clients[6].m_game_launch_event->player_index);

  // The game channel must be the same.
  EXPECT_EQ(m_clients[4].m_game_launch_event->channel,
            m_clients[5].m_game_launch_event->channel);
  EXPECT_EQ(m_clients[4].m_game_launch_event->channel,
            m_clients[6].m_game_launch_event->channel);

  // The features must be the same.
  EXPECT_EQ(bim::game::feature_flags(0b1110'0000),
            m_clients[4].m_game_launch_event->features);
  EXPECT_EQ(m_clients[4].m_game_launch_event->features,
            m_clients[5].m_game_launch_event->features);
  EXPECT_EQ(m_clients[4].m_game_launch_event->features,
            m_clients[6].m_game_launch_event->features);

  // The game channel of each group of players must be different.
  EXPECT_NE(m_clients[0].m_game_launch_event->channel,
            m_clients[4].m_game_launch_event->channel);
}

/**
 * Two groups of players ask for a random game, one group after the other, two
 * games must be created.
 */
TEST_F(game_creation_test, two_random_games)
{
  for (int i = 0; i != 7; ++i)
    m_clients[i].authenticate();

  for (int i = 0; i != 4; ++i)
    m_clients[i].new_game(bim::game::feature_flags(0b11 | (1 << i)));

  // Let the time pass such that the messages can move between the clients and
  // the server.
  for (int i = 0; i != 10; ++i)
    m_scheduler.tick(std::chrono::seconds(1));

  // The first four players must be in the same game.
  for (int i = 0; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].m_game_launch_event) << "i=" << i;
      EXPECT_EQ(4, m_clients[i].m_game_launch_event->player_count)
          << "i=" << i;
    }

  // The player index must all be different.
  EXPECT_NE(m_clients[0].m_game_launch_event->player_index,
            m_clients[1].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[0].m_game_launch_event->player_index,
            m_clients[2].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[0].m_game_launch_event->player_index,
            m_clients[3].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[1].m_game_launch_event->player_index,
            m_clients[2].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[1].m_game_launch_event->player_index,
            m_clients[3].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[2].m_game_launch_event->player_index,
            m_clients[3].m_game_launch_event->player_index);

  // The game channel must be the same.
  EXPECT_EQ(m_clients[0].m_game_launch_event->channel,
            m_clients[1].m_game_launch_event->channel);
  EXPECT_EQ(m_clients[0].m_game_launch_event->channel,
            m_clients[2].m_game_launch_event->channel);
  EXPECT_EQ(m_clients[0].m_game_launch_event->channel,
            m_clients[3].m_game_launch_event->channel);

  // The features must be the same.
  EXPECT_EQ(bim::game::feature_flags(0b11),
            m_clients[0].m_game_launch_event->features);
  EXPECT_EQ(m_clients[0].m_game_launch_event->features,
            m_clients[1].m_game_launch_event->features);
  EXPECT_EQ(m_clients[0].m_game_launch_event->features,
            m_clients[2].m_game_launch_event->features);
  EXPECT_EQ(m_clients[0].m_game_launch_event->features,
            m_clients[3].m_game_launch_event->features);

  // Second group as for a random game
  for (int i = 4; i != 7; ++i)
    m_clients[i].new_game(bim::game::feature_flags(0b1100'0000 | (1 << i)));

  // Time passes again…
  for (int i = 0; i != 10; ++i)
    m_scheduler.tick(std::chrono::seconds(1));

  for (int i = 4; i != 7; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].m_game_launch_event) << "i=" << i;
      EXPECT_EQ(3, m_clients[i].m_game_launch_event->player_count)
          << "i=" << i;
    }

  // The player index must all be different.
  EXPECT_NE(m_clients[4].m_game_launch_event->player_index,
            m_clients[5].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[4].m_game_launch_event->player_index,
            m_clients[6].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[5].m_game_launch_event->player_index,
            m_clients[6].m_game_launch_event->player_index);

  // The game channel must be the same.
  EXPECT_EQ(m_clients[4].m_game_launch_event->channel,
            m_clients[5].m_game_launch_event->channel);
  EXPECT_EQ(m_clients[4].m_game_launch_event->channel,
            m_clients[6].m_game_launch_event->channel);

  // The features must be the same.
  EXPECT_EQ(bim::game::feature_flags(0b1100'0000),
            m_clients[4].m_game_launch_event->features);
  EXPECT_EQ(m_clients[4].m_game_launch_event->features,
            m_clients[5].m_game_launch_event->features);
  EXPECT_EQ(m_clients[4].m_game_launch_event->features,
            m_clients[6].m_game_launch_event->features);

  // The game channel of each group of players must be different.
  EXPECT_NE(m_clients[0].m_game_launch_event->channel,
            m_clients[4].m_game_launch_event->channel);
}

/**
 * Two groups of players simultaneously ask for a random game, two games must
 * be created.
 */
TEST_F(game_creation_test, two_random_games_simultaneously)
{
  for (int i = 0; i != 7; ++i)
    m_clients[i].authenticate();

  for (int i = 0; i != 7; ++i)
    m_clients[i].new_game({});

  // Let the time pass such that the messages can move between the clients and
  // the server.
  for (int i = 0; i != 10; ++i)
    m_scheduler.tick(std::chrono::seconds(1));

  std::unordered_map<iscool::net::channel_id, int> player_count_per_channel;

  // All players must be in a game.
  for (int i = 0; i != 7; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].m_game_launch_event) << "i=" << i;
      ++player_count_per_channel[m_clients[i].m_game_launch_event->channel];
    }

  // There must be two games, of four players each.
  ASSERT_EQ(2, player_count_per_channel.size());

  if (player_count_per_channel.begin()->second == 4)
    EXPECT_EQ(3, std::next(player_count_per_channel.begin())->second);
  else
    {
      EXPECT_EQ(3, player_count_per_channel.begin()->second);
      EXPECT_EQ(4, std::next(player_count_per_channel.begin())->second);
    }
}

/**
 * Players asking for a named game are put together, those asking for a random
 * game are put in another group.
 */
TEST_F(game_creation_test, mix_random_and_named_games)
{
  for (int i = 0; i != 7; ++i)
    m_clients[i].authenticate();

  for (int i = 0; i != 4; ++i)
    m_clients[i].new_game(bim::game::feature_flags(0b1010 | (1 << i)));

  for (int i = 4; i != 7; ++i)
    m_clients[i].new_game(
        { 'g', 'a', 'm', 'e' },
        bim::game::feature_flags(0b110'0000 | (1 << (i - 4))));

  // Let the time pass such that the messages can move between the clients and
  // the server.
  for (int i = 0; i != 10; ++i)
    m_scheduler.tick(std::chrono::seconds(1));

  // Time passes again…
  for (int i = 0; i != 10; ++i)
    m_scheduler.tick(std::chrono::seconds(1));

  for (int i = 0; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].m_game_launch_event) << "i=" << i;
      EXPECT_EQ(4, m_clients[i].m_game_launch_event->player_count)
          << "i=" << i;
    }

  // The player index must all be different.
  EXPECT_NE(m_clients[0].m_game_launch_event->player_index,
            m_clients[1].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[0].m_game_launch_event->player_index,
            m_clients[2].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[0].m_game_launch_event->player_index,
            m_clients[3].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[1].m_game_launch_event->player_index,
            m_clients[2].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[1].m_game_launch_event->player_index,
            m_clients[3].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[2].m_game_launch_event->player_index,
            m_clients[3].m_game_launch_event->player_index);

  // The game channel must be the same.
  EXPECT_EQ(m_clients[0].m_game_launch_event->channel,
            m_clients[1].m_game_launch_event->channel);
  EXPECT_EQ(m_clients[0].m_game_launch_event->channel,
            m_clients[2].m_game_launch_event->channel);
  EXPECT_EQ(m_clients[0].m_game_launch_event->channel,
            m_clients[3].m_game_launch_event->channel);

  // The features must be the same.
  EXPECT_EQ(bim::game::feature_flags(0b1010),
            m_clients[0].m_game_launch_event->features);
  EXPECT_EQ(m_clients[0].m_game_launch_event->features,
            m_clients[1].m_game_launch_event->features);
  EXPECT_EQ(m_clients[0].m_game_launch_event->features,
            m_clients[2].m_game_launch_event->features);
  EXPECT_EQ(m_clients[0].m_game_launch_event->features,
            m_clients[3].m_game_launch_event->features);

  for (int i = 4; i != 7; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].m_game_launch_event) << "i=" << i;
      EXPECT_EQ(3, m_clients[i].m_game_launch_event->player_count)
          << "i=" << i;
    }

  // The player index must all be different.
  EXPECT_NE(m_clients[4].m_game_launch_event->player_index,
            m_clients[5].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[4].m_game_launch_event->player_index,
            m_clients[6].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[5].m_game_launch_event->player_index,
            m_clients[6].m_game_launch_event->player_index);

  // The game channel must be the same.
  EXPECT_EQ(m_clients[4].m_game_launch_event->channel,
            m_clients[5].m_game_launch_event->channel);
  EXPECT_EQ(m_clients[4].m_game_launch_event->channel,
            m_clients[6].m_game_launch_event->channel);

  // The features must be the same.
  EXPECT_EQ(bim::game::feature_flags(0b110'0000),
            m_clients[4].m_game_launch_event->features);
  EXPECT_EQ(m_clients[4].m_game_launch_event->features,
            m_clients[5].m_game_launch_event->features);
  EXPECT_EQ(m_clients[4].m_game_launch_event->features,
            m_clients[6].m_game_launch_event->features);

  // The game channel of each group of players must be different.
  EXPECT_NE(m_clients[0].m_game_launch_event->channel,
            m_clients[4].m_game_launch_event->channel);
}

void game_creation_test::validate_player_order()
{
  // Let the time pass such that the messages can move between the clients and
  // the server.
  for (int i = 0; i != 10; ++i)
    m_scheduler.tick(std::chrono::seconds(1));

  // The four players must be in the same game.
  for (int i = 0; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].m_game_launch_event) << "i=" << i;
      EXPECT_EQ(4, m_clients[i].m_game_launch_event->player_count)
          << "i=" << i;
      ASSERT_LT(m_clients[i].m_game_launch_event->player_index, 4)
          << "i=" << i;
    }

  // The player index must all be different.
  EXPECT_NE(m_clients[0].m_game_launch_event->player_index,
            m_clients[1].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[0].m_game_launch_event->player_index,
            m_clients[2].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[0].m_game_launch_event->player_index,
            m_clients[3].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[1].m_game_launch_event->player_index,
            m_clients[2].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[1].m_game_launch_event->player_index,
            m_clients[3].m_game_launch_event->player_index);
  EXPECT_NE(m_clients[2].m_game_launch_event->player_index,
            m_clients[3].m_game_launch_event->player_index);

  // The players must be orderered by increasing session.
  iscool::net::session_id sessions[4];

  for (int i = 0; i != 4; ++i)
    sessions[m_clients[i].m_game_launch_event->player_index] =
        *m_clients[i].m_session;

  EXPECT_LT(sessions[0], sessions[1]);
  EXPECT_LT(sessions[1], sessions[2]);
  EXPECT_LT(sessions[2], sessions[3]);
}

/**
 * Players in a random game must be ordered by increasing order of session.
 */
TEST_F(game_creation_test, player_order_named_games)
{
  for (int i = 0; i != 4; ++i)
    m_clients[i].authenticate();

  const bim::net::game_name& game_name = { 'g', 'a', 'm', 'e', '1' };
  m_clients[2].new_game(game_name, {});
  m_clients[0].new_game(game_name, {});
  m_clients[3].new_game(game_name, {});
  m_clients[1].new_game(game_name, {});

  validate_player_order();
}

/**
 * Players in a named game must be ordered by increasing order of session.
 */
TEST_F(game_creation_test, player_order_random_games)
{
  for (int i = 0; i != 4; ++i)
    m_clients[i].authenticate();

  m_clients[3].new_game({});
  m_clients[0].new_game({});
  m_clients[2].new_game({});
  m_clients[1].new_game({});

  validate_player_order();
}
