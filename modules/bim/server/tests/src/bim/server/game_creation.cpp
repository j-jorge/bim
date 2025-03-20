// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/fake_scheduler.hpp>

#include <bim/server/tests/new_test_config.hpp>

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
    void new_game_auto_accept(const bim::net::game_name& name,
                              bim::game::feature_flags features);
    void new_game_auto_accept(bim::game::feature_flags features);
    void new_game(const bim::net::game_name& name);
    void new_game();

    void accept_game(bim::game::feature_flags features);

  public:
    std::optional<bim::net::game_launch_event> m_game_launch_event;

  private:
    void launch_game(const bim::net::game_launch_event& event);

  private:
    bim::server::tests::fake_scheduler& m_scheduler;

    bim::net::authentication_exchange m_authentication;
    bim::net::new_game_exchange m_new_game;

    std::optional<iscool::net::session_id> m_session;
  };

public:
  game_creation_test();

protected:
  iscool::log::scoped_initializer m_log;
  bim::server::tests::fake_scheduler m_scheduler;

  const bim::server::config m_config;
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
      [this](iscool::net::session_id session) -> void
      {
        m_session = session;
      });

  m_authentication.connect_to_error(
      [](bim::net::authentication_error_code) -> void
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

void game_creation_test::client::new_game_auto_accept(
    const bim::net::game_name& name, bim::game::feature_flags features)
{
  ASSERT_TRUE(!!m_session);

  m_new_game.connect_to_game_proposal(
      std::bind(&bim::net::new_game_exchange::accept, &m_new_game, features));

  m_new_game.start(*m_session, name);
}

void game_creation_test::client::new_game_auto_accept(
    bim::game::feature_flags features)
{
  ASSERT_TRUE(!!m_session);

  m_new_game.connect_to_game_proposal(
      std::bind(&bim::net::new_game_exchange::accept, &m_new_game, features));
  m_new_game.start(*m_session);
}

void game_creation_test::client::new_game(const bim::net::game_name& name)
{
  ASSERT_TRUE(!!m_session);

  m_new_game.start(*m_session, name);
}

void game_creation_test::client::new_game()
{
  ASSERT_TRUE(!!m_session);

  m_new_game.start(*m_session);
}

void game_creation_test::client::accept_game(bim::game::feature_flags features)
{
  m_new_game.accept(features);
}

void game_creation_test::client::launch_game(
    const bim::net::game_launch_event& event)
{
  EXPECT_FALSE(!!m_game_launch_event);
  m_game_launch_event = event;
}

game_creation_test::game_creation_test()
  : m_config(
        []() -> bim::server::config
        {
          bim::server::config config = bim::server::tests::new_test_config();
          // Short delay for the tests where one player never accept the game.
          config.random_game_auto_start_delay = std::chrono::seconds(10);
          return config;
        }())
  , m_server(m_config)
  , m_socket_stream("localhost:" + std::to_string(m_config.port),
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
    m_clients[i].new_game_auto_accept(
        { 'g', 'a', 'm', 'e', '1' },
        bim::game::feature_flags(0b1110 | (1 << i)));

  // Let the time pass such that the messages can move between the clients and
  // the server.
  for (int i = 0; i != 10; ++i)
    m_scheduler.tick(std::chrono::seconds(1));

  for (int i = 4; i != 7; ++i)
    m_clients[i].new_game_auto_accept(
        { 'g', 'a', 'm', 'e', '2' },
        bim::game::feature_flags(0b1110'0000 | (1 << i)));

  // Time passes again…
  for (int i = 0; i != 10; ++i)
    m_scheduler.tick(std::chrono::seconds(1));

  for (int i = 0; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].m_game_launch_event) << "i=" << i;
      EXPECT_EQ(4, m_clients[i].m_game_launch_event->fingerprint.player_count)
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
            m_clients[0].m_game_launch_event->fingerprint.features);
  EXPECT_EQ(m_clients[0].m_game_launch_event->fingerprint.features,
            m_clients[1].m_game_launch_event->fingerprint.features);
  EXPECT_EQ(m_clients[0].m_game_launch_event->fingerprint.features,
            m_clients[2].m_game_launch_event->fingerprint.features);
  EXPECT_EQ(m_clients[0].m_game_launch_event->fingerprint.features,
            m_clients[3].m_game_launch_event->fingerprint.features);

  for (int i = 4; i != 7; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].m_game_launch_event) << "i=" << i;
      EXPECT_EQ(3, m_clients[i].m_game_launch_event->fingerprint.player_count)
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
            m_clients[4].m_game_launch_event->fingerprint.features);
  EXPECT_EQ(m_clients[4].m_game_launch_event->fingerprint.features,
            m_clients[5].m_game_launch_event->fingerprint.features);
  EXPECT_EQ(m_clients[4].m_game_launch_event->fingerprint.features,
            m_clients[6].m_game_launch_event->fingerprint.features);

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
    m_clients[i].new_game_auto_accept(
        bim::game::feature_flags(0b11 | (1 << i)));

  // Let the time pass such that the messages can move between the clients and
  // the server.
  for (int i = 0; i != 10; ++i)
    m_scheduler.tick(std::chrono::seconds(1));

  // The first four players must be in the same game.
  for (int i = 0; i != 4; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].m_game_launch_event) << "i=" << i;
      EXPECT_EQ(4, m_clients[i].m_game_launch_event->fingerprint.player_count)
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
            m_clients[0].m_game_launch_event->fingerprint.features);
  EXPECT_EQ(m_clients[0].m_game_launch_event->fingerprint.features,
            m_clients[1].m_game_launch_event->fingerprint.features);
  EXPECT_EQ(m_clients[0].m_game_launch_event->fingerprint.features,
            m_clients[2].m_game_launch_event->fingerprint.features);
  EXPECT_EQ(m_clients[0].m_game_launch_event->fingerprint.features,
            m_clients[3].m_game_launch_event->fingerprint.features);

  // Second group as for a random game
  for (int i = 4; i != 7; ++i)
    m_clients[i].new_game_auto_accept(
        bim::game::feature_flags(0b1100'0000 | (1 << i)));

  // Time passes again…
  for (int i = 0; i != 10; ++i)
    m_scheduler.tick(std::chrono::seconds(1));

  for (int i = 4; i != 7; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].m_game_launch_event) << "i=" << i;
      EXPECT_EQ(3, m_clients[i].m_game_launch_event->fingerprint.player_count)
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
            m_clients[4].m_game_launch_event->fingerprint.features);
  EXPECT_EQ(m_clients[4].m_game_launch_event->fingerprint.features,
            m_clients[5].m_game_launch_event->fingerprint.features);
  EXPECT_EQ(m_clients[4].m_game_launch_event->fingerprint.features,
            m_clients[6].m_game_launch_event->fingerprint.features);

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
    m_clients[i].new_game_auto_accept({});

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
    m_clients[i].new_game_auto_accept(
        bim::game::feature_flags(0b1010 | (1 << i)));

  for (int i = 4; i != 7; ++i)
    m_clients[i].new_game_auto_accept(
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
      EXPECT_EQ(4, m_clients[i].m_game_launch_event->fingerprint.player_count)
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
            m_clients[0].m_game_launch_event->fingerprint.features);
  EXPECT_EQ(m_clients[0].m_game_launch_event->fingerprint.features,
            m_clients[1].m_game_launch_event->fingerprint.features);
  EXPECT_EQ(m_clients[0].m_game_launch_event->fingerprint.features,
            m_clients[2].m_game_launch_event->fingerprint.features);
  EXPECT_EQ(m_clients[0].m_game_launch_event->fingerprint.features,
            m_clients[3].m_game_launch_event->fingerprint.features);

  for (int i = 4; i != 7; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].m_game_launch_event) << "i=" << i;
      EXPECT_EQ(3, m_clients[i].m_game_launch_event->fingerprint.player_count)
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
            m_clients[4].m_game_launch_event->fingerprint.features);
  EXPECT_EQ(m_clients[4].m_game_launch_event->fingerprint.features,
            m_clients[5].m_game_launch_event->fingerprint.features);
  EXPECT_EQ(m_clients[4].m_game_launch_event->fingerprint.features,
            m_clients[6].m_game_launch_event->fingerprint.features);

  // The game channel of each group of players must be different.
  EXPECT_NE(m_clients[0].m_game_launch_event->channel,
            m_clients[4].m_game_launch_event->channel);
}

/**
 * One player never accepts the random game, it should be created anyway with
 * the other players.
 */
TEST_F(game_creation_test, random_game_ignore_never_accepting_player)
{
  for (int i = 0; i != 4; ++i)
    m_clients[i].authenticate();

  constexpr int inactive_player_index = 2;

  for (int i = 0; i != 4; ++i)
    if (i != inactive_player_index)
      m_clients[i].new_game_auto_accept({});

  m_clients[inactive_player_index].new_game();

  // Let the time pass such that the messages can move between the clients and
  // the server.
  for (int i = 0; i != 100; ++i)
    m_scheduler.tick(std::chrono::seconds(1));

  // All players except the inactive one must be in a game.
  EXPECT_TRUE(!m_clients[inactive_player_index].m_game_launch_event);

  for (int i = 0; i != 4; ++i)
    if (i != inactive_player_index)
      {
        ASSERT_TRUE(!!m_clients[i].m_game_launch_event) << "i=" << i;
      }

  for (int i = 0; i != 4; ++i)
    if (i != inactive_player_index)
      {
        EXPECT_EQ(m_clients[0].m_game_launch_event->channel,
                  m_clients[i].m_game_launch_event->channel)
            << "i=" << i;
      }
}

/**
 * One player never accepts the named game, we should wait.
 */
TEST_F(game_creation_test, named_game_ignore_never_accepting_player)
{
  for (int i = 0; i != 4; ++i)
    m_clients[i].authenticate();

  const bim::net::game_name name{ 'i', 'n', 'a', 'c', 't', 'i', 'v', 'e' };
  constexpr int inactive_player_index = 2;

  for (int i = 0; i != 4; ++i)
    if (i != inactive_player_index)
      m_clients[i].new_game_auto_accept(name, {});

  m_clients[inactive_player_index].new_game(name);

  // Let the time pass such that the messages can move between the clients and
  // the server.
  for (int i = 0; i != 100; ++i)
    m_scheduler.tick(std::chrono::seconds(1));

  // The game should not have started.
  for (int i = 0; i != 4; ++i)
    EXPECT_TRUE(!m_clients[i].m_game_launch_event) << "i=" << i;

  m_clients[inactive_player_index].accept_game({});

  for (int i = 0; i != 100; ++i)
    m_scheduler.tick(std::chrono::seconds(1));

  // The game should have started with all players.
  for (int i = 0; i != 4; ++i)
    ASSERT_TRUE(!!m_clients[i].m_game_launch_event) << "i=" << i;

  for (int i = 0; i != 4; ++i)
    EXPECT_EQ(m_clients[0].m_game_launch_event->channel,
              m_clients[i].m_game_launch_event->channel)
        << "i=" << i;
}
