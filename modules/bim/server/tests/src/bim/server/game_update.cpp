// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/fake_scheduler.hpp>

#include <bim/server/config.hpp>
#include <bim/server/server.hpp>

#include <bim/net/exchange/authentication_exchange.hpp>
#include <bim/net/exchange/game_launch_event.hpp>
#include <bim/net/exchange/game_update_exchange.hpp>
#include <bim/net/exchange/new_game_exchange.hpp>

#include <bim/game/component/player_movement.hpp>

#include <iscool/log/setup.hpp>
#include <iscool/net/message_channel.hpp>
#include <iscool/signals/scoped_connection.hpp>

#include <optional>

#include <gtest/gtest.h>

class game_update_test : public testing::TestWithParam<int>
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
    std::optional<bool> m_started;
    std::unique_ptr<bim::net::game_update_exchange> m_game_update;

    bim::net::server_update m_all_updates;

  private:
    void launch_game(iscool::net::message_stream& stream,
                     const bim::net::game_launch_event& event);

  private:
    bim::server::tests::fake_scheduler& m_scheduler;

    bim::net::authentication_exchange m_authentication;
    bim::net::new_game_exchange m_new_game;
    std::unique_ptr<iscool::net::message_channel> m_message_channel;

    std::optional<iscool::net::session_id> m_session;
  };

public:
  game_update_test();

protected:
  void join_game(int player_count, const bim::net::game_name& game_name);
  void wait();
  void wait(const std::function<bool()>& ready);

protected:
  iscool::log::scoped_initializer m_log;
  bim::server::tests::fake_scheduler m_scheduler;

  const unsigned short m_port;
  bim::server::server m_server;
  iscool::net::socket_stream m_socket_stream;
  iscool::net::message_stream m_message_stream;

  std::array<client, 4> m_clients;
};

game_update_test::client::client(bim::server::tests::fake_scheduler& scheduler,
                                 iscool::net::message_stream& message_stream)
  : m_scheduler(scheduler)
  , m_authentication(message_stream)
  , m_new_game(message_stream)
{
  m_all_updates.from_tick = 0;

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

  m_new_game.connect_to_game_proposal(
      std::bind(&bim::net::new_game_exchange::accept, &m_new_game,
                bim::game::feature_flags{}));

  m_new_game.connect_to_launch_game(std::bind(&client::launch_game, this,
                                              std::ref(message_stream),
                                              std::placeholders::_1));
}

void game_update_test::client::authenticate()
{
  EXPECT_FALSE(!!m_session);

  m_authentication.start();

  for (int i = 0; (i != 10) && !m_session; ++i)
    m_scheduler.tick(std::chrono::seconds(1));
}

void game_update_test::client::new_game(const bim::net::game_name& name)
{
  ASSERT_TRUE(!!m_session);

  m_new_game.start(*m_session, name);
}

void game_update_test::client::launch_game(
    iscool::net::message_stream& stream,
    const bim::net::game_launch_event& event)
{
  EXPECT_TRUE(!!m_session);

  m_message_channel.reset(
      new iscool::net::message_channel(stream, *m_session, event.channel));
  m_game_update.reset(new bim::net::game_update_exchange(*m_message_channel,
                                                         event.player_count));

  m_game_update->connect_to_started(
      [this]() -> void
      {
        m_started.emplace(true);
      });
  m_game_update->connect_to_updated(
      [this, player_count = event.player_count](
          const bim::net::server_update& update) -> void
      {
        for (std::size_t i = 0; i != player_count; ++i)
          {
            EXPECT_EQ(m_all_updates.from_tick
                          + m_all_updates.actions[i].size(),
                      update.from_tick)
                << "i=" << i
                << ", m_all_updates.from_tick=" << m_all_updates.from_tick
                << ", m_all_updates.actions[i].size()="
                << m_all_updates.actions[i].size();

            m_all_updates.actions[i].insert(m_all_updates.actions[i].end(),
                                            update.actions[i].begin(),
                                            update.actions[i].end());
          }

        for (std::size_t i = player_count; i != m_all_updates.actions.size();
             ++i)
          EXPECT_TRUE(m_all_updates.actions[i].empty()) << "i=" << i;
      });

  m_game_update->start();
}

game_update_test::game_update_test()
  : m_port(10004)
  , m_server(bim::server::config(m_port))
  , m_socket_stream("localhost:" + std::to_string(m_port),
                    iscool::net::socket_mode::client{})
  , m_message_stream(m_socket_stream)
  , m_clients{ client(m_scheduler, m_message_stream),
               client(m_scheduler, m_message_stream),
               client(m_scheduler, m_message_stream),
               client(m_scheduler, m_message_stream) }
{}

void game_update_test::join_game(int player_count,
                                 const bim::net::game_name& game_name)
{
  for (int i = 0; i != player_count; ++i)
    m_clients[i].authenticate();

  for (int i = 0; i != player_count; ++i)
    m_clients[i].new_game(game_name);

  // Let the time pass such that the messages can move between the clients and
  // the server.
  wait(
      [this, player_count]() -> bool
      {
        for (int i = 0; i != player_count; ++i)
          if (!m_clients[i].m_started)
            return false;

        return true;
      });
}

void game_update_test::wait()
{
  wait(
      []() -> bool
      {
        return false;
      });
}

void game_update_test::wait(const std::function<bool()>& ready)
{
  for (int i = 0; i != 500; ++i)
    {
      std::this_thread::sleep_for(std::chrono::seconds(0));
      m_scheduler.tick(std::chrono::milliseconds(20));

      if (ready())
        break;
    }
}

/** The game should begin with a bim::net::start message. */
TEST_P(game_update_test, start)
{
  const int player_count = GetParam();

  join_game(player_count, { 'u', 'p', 'd', 'a', 't', 'e', '1' });

  for (int i = 0; i != player_count; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].m_started) << "i=" << i;
      EXPECT_TRUE(*m_clients[i].m_started) << "i=" << i;
    }
}

/**
 * The server should broadcast the actions up to the highest tick reached by
 * all players.
 */
TEST_P(game_update_test, game_instant)
{
  const int player_count = GetParam();

  join_game(player_count, { 'u', 'p', 'd', 'a', 't', 'e', '2' });

  for (int i = 0; i != player_count; ++i)
    ASSERT_TRUE(!!m_clients[i].m_started) << "i=" << i;

  const bim::game::player_action actions[] = {
    bim::game::player_action{ .movement = bim::game::player_movement::up,
                              .drop_bomb = false },
    bim::game::player_action{ .movement = bim::game::player_movement::up,
                              .drop_bomb = true },
    bim::game::player_action{ .movement = bim::game::player_movement::idle,
                              .drop_bomb = true },
    bim::game::player_action{ .movement = bim::game::player_movement::down,
                              .drop_bomb = false }
  };

  for (int i = 0; i != player_count; ++i)
    m_clients[i].m_game_update->push(actions[i]);

  wait();

  // Test the state observed by each player.
  for (int i = 0; i != player_count; ++i)
    {
      EXPECT_EQ(0, m_clients[i].m_all_updates.from_tick) << "i=" << i;

      // Each player sees a state for all players.
      for (int player_index = 0; player_index != player_count; ++player_index)
        {
          const std::vector<bim::game::player_action>& server_actions =
              m_clients[i].m_all_updates.actions[player_index];

          ASSERT_EQ(1, server_actions.size())
              << "i=" << i << ", player_index=" << player_index;

          EXPECT_EQ(actions[player_index].movement, server_actions[0].movement)
              << "i=" << i << ", player_index=" << player_index;
          EXPECT_EQ(actions[player_index].drop_bomb,
                    server_actions[0].drop_bomb)
              << "i=" << i << ", player_index=" << player_index;
        }
    }
}

/**
 * The updates from the server must match the highest tick reached by all
 * players. If a player is late, the highest tick is his progress point.
 */
TEST_P(game_update_test, player_two_is_late)
{
  const int player_count = GetParam();

  join_game(player_count, { 'l', 'a', 't', 'e' });

  constexpr int tick_count = 5;
  const bim::game::player_movement movements[4][tick_count] = {
    {
        bim::game::player_movement::up,
        bim::game::player_movement::down,
        bim::game::player_movement::left,
        bim::game::player_movement::right,
        bim::game::player_movement::up,
    },
    { bim::game::player_movement::down, bim::game::player_movement::left,
      bim::game::player_movement::right, bim::game::player_movement::up,
      bim::game::player_movement::down },
    { bim::game::player_movement::left, bim::game::player_movement::right,
      bim::game::player_movement::up, bim::game::player_movement::down,
      bim::game::player_movement::left },
    { bim::game::player_movement::right, bim::game::player_movement::up,
      bim::game::player_movement::down, bim::game::player_movement::left,
      bim::game::player_movement::right }
  };

  // Every player sends an action.
  for (int client_index = 0; client_index != player_count; ++client_index)
    m_clients[client_index].m_game_update->push(bim::game::player_action{
        .movement = movements[client_index][0], .drop_bomb = false });

  wait();

  const auto check_actions = [=, this](std::uint32_t expected_tick)
  {
    for (int client_index = 0; client_index != player_count; ++client_index)
      {
        EXPECT_EQ(expected_tick,
                  m_clients[client_index].m_all_updates.from_tick)
            << "client_index=" << client_index;
        ASSERT_EQ(4, m_clients[client_index].m_all_updates.actions.size())
            << "client_index=" << client_index;

        // Each player sees a state for all players.
        for (int player_index = 0; player_index != player_count;
             ++player_index)
          {
            const std::vector<bim::game::player_action>& server_actions =
                m_clients[client_index].m_all_updates.actions[player_index];

            ASSERT_EQ(expected_tick + 1, server_actions.size())
                << "client_index=" << client_index
                << ", player_index=" << player_index;
            EXPECT_EQ(movements[player_index][expected_tick],
                      server_actions[expected_tick].movement)
                << "client_index=" << client_index
                << ", player_index=" << player_index;
            EXPECT_FALSE(server_actions[expected_tick].drop_bomb)
                << "client_index=" << client_index
                << ", player_index=" << player_index;
          }
      }
  };

  // Test the state observed by each player.
  check_actions(0);

  // Every player except index=1 sends an action.
  for (int tick = 1; tick != tick_count; ++tick)
    {
      for (int client_index = 0; client_index != player_count; ++client_index)
        if (client_index != 1)
          m_clients[client_index].m_game_update->push(bim::game::player_action{
              .movement = movements[client_index][tick], .drop_bomb = false });

      wait();

      // Should still be at frame 0
      check_actions(0);
    }

  // Suddenly, player index=1 sends his actions.
  for (int tick = 1; tick != tick_count; ++tick)
    m_clients[1].m_game_update->push(bim::game::player_action{
        .movement = movements[1][tick], .drop_bomb = false });

  wait();

  // All clients should have received the three last ticks.
  for (int client_index = 0; client_index != player_count; ++client_index)
    {
      ASSERT_EQ(4, m_clients[client_index].m_all_updates.actions.size())
          << "client_index=" << client_index;

      for (int player_index = 0; player_index != player_count; ++player_index)
        {
          const std::vector<bim::game::player_action>& server_actions =
              m_clients[client_index].m_all_updates.actions[player_index];

          ASSERT_EQ(tick_count, server_actions.size())
              << "client_index=" << client_index
              << ", player_index=" << player_index;

          // Each player sees a state for all players.
          for (int frame_index = 1; frame_index != tick_count; ++frame_index)
            {
              EXPECT_EQ(movements[player_index][frame_index],
                        server_actions[frame_index].movement)
                  << "client_index=" << client_index
                  << ", player_index=" << player_index
                  << ", frame_index=" << frame_index;
              EXPECT_FALSE(server_actions[frame_index].drop_bomb)
                  << "client_index=" << client_index
                  << ", player_index=" << player_index
                  << ", frame_index=" << frame_index;
            }
        }
    }
}

INSTANTIATE_TEST_SUITE_P(game_update_test_instance, game_update_test,
                         testing::Values(2, 3, 4));
