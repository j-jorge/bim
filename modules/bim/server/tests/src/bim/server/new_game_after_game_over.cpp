// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/fake_scheduler.hpp>

#include <bim/server/server.hpp>

#include <bim/net/contest_runner.hpp>
#include <bim/net/exchange/authentication_exchange.hpp>
#include <bim/net/exchange/game_launch_event.hpp>
#include <bim/net/exchange/game_update_exchange.hpp>
#include <bim/net/exchange/new_game_exchange.hpp>

#include <bim/game/component/player_action_queue.hpp>
#include <bim/game/component/player_movement.hpp>
#include <bim/game/contest.hpp>
#include <bim/game/player_action.hpp>

#include <iscool/log/setup.hpp>
#include <iscool/net/message_channel.hpp>
#include <iscool/signals/scoped_connection.hpp>

#include <optional>

#include <gtest/gtest.h>

class new_game_after_game_over_test : public testing::TestWithParam<int>
{
protected:
  class client
  {
  public:
    client(bim::server::tests::fake_scheduler& scheduler,
           iscool::net::message_stream& message_stream);

    void authenticate();
    void new_game();
    void tick(std::chrono::nanoseconds d);

  public:
    std::optional<bool> m_started;
    std::unique_ptr<bim::net::contest_runner> m_contest_runner;
    bim::game::contest_result m_result;
    bim::game::player_action* m_action;
    int m_player_index;

  private:
    void launch_game(iscool::net::message_stream& stream,
                     const bim::net::game_launch_event& event);

  private:
    bim::server::tests::fake_scheduler& m_scheduler;

    bim::net::authentication_exchange m_authentication;
    bim::net::new_game_exchange m_new_game;
    std::unique_ptr<iscool::net::message_channel> m_message_channel;

    std::optional<iscool::net::session_id> m_session;

    std::unique_ptr<bim::net::game_update_exchange> m_game_update;
    std::unique_ptr<bim::game::contest> m_contest;
  };

public:
  new_game_after_game_over_test();

protected:
  void authenticate();
  void join_game();
  void tick();
  void tick(std::chrono::nanoseconds d);
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

new_game_after_game_over_test::client::client(
    bim::server::tests::fake_scheduler& scheduler,
    iscool::net::message_stream& message_stream)
  : m_action(nullptr)
  , m_scheduler(scheduler)
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

  m_new_game.connect_to_game_proposal(
      std::bind(&bim::net::new_game_exchange::accept, &m_new_game));

  m_new_game.connect_to_launch_game(std::bind(&client::launch_game, this,
                                              std::ref(message_stream),
                                              std::placeholders::_1));
}

void new_game_after_game_over_test::client::authenticate()
{
  EXPECT_FALSE(!!m_session);

  m_authentication.start();

  for (int i = 0; (i != 10) && !m_session; ++i)
    m_scheduler.tick(std::chrono::milliseconds(20));
}

void new_game_after_game_over_test::client::new_game()
{
  ASSERT_TRUE(!!m_session);

  m_message_channel.reset();
  m_game_update.reset();
  m_contest.reset();
  m_action = nullptr;
  m_started = std::nullopt;

  m_new_game.start(*m_session);
}

void new_game_after_game_over_test::client::launch_game(
    iscool::net::message_stream& stream,
    const bim::net::game_launch_event& event)
{
  EXPECT_TRUE(!!m_session);

  m_player_index = event.player_index;

  m_message_channel.reset(
      new iscool::net::message_channel(stream, *m_session, event.channel));
  m_game_update.reset(new bim::net::game_update_exchange(*m_message_channel,
                                                         event.player_count));
  m_contest.reset(
      new bim::game::contest(event.seed, 80, event.player_count, 13, 15));
  m_contest_runner.reset(new bim::net::contest_runner(
      *m_contest, *m_game_update, event.player_index, event.player_count));

  m_action = bim::game::find_player_action_by_index(m_contest->registry(),
                                                    m_player_index);

  m_game_update->connect_to_started(
      [this]() -> void
      {
        m_started.emplace(true);
      });

  m_game_update->start();
}

void new_game_after_game_over_test::client::tick(std::chrono::nanoseconds d)
{
  ASSERT_NE(nullptr, m_contest_runner);
  m_result = m_contest_runner->run(d);
}

new_game_after_game_over_test::new_game_after_game_over_test()
  : m_port(10005)
  , m_server(m_port)
  , m_socket_stream("localhost:" + std::to_string(m_port),
                    iscool::net::socket_mode::client{})
  , m_message_stream(m_socket_stream)
  , m_clients{ client(m_scheduler, m_message_stream),
               client(m_scheduler, m_message_stream),
               client(m_scheduler, m_message_stream),
               client(m_scheduler, m_message_stream) }
{}

void new_game_after_game_over_test::authenticate()
{
  const int player_count = GetParam();

  for (int i = 0; i != player_count; ++i)
    m_clients[i].authenticate();
}

void new_game_after_game_over_test::join_game()
{
  const int player_count = GetParam();

  for (int i = 0; i != player_count; ++i)
    m_clients[i].new_game();

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

void new_game_after_game_over_test::tick(std::chrono::nanoseconds d)
{
  const int player_count = GetParam();

  const std::size_t tick_count = d / bim::game::contest::tick_interval
                                 + (d % bim::game::contest::tick_interval
                                    != std::chrono::nanoseconds::zero());

  std::array<std::size_t, 4> expected_tick;

  for (int i = 0; i != player_count; ++i)
    expected_tick[i] =
        m_clients[i].m_contest_runner->confirmed_tick() + tick_count;

  const auto all_synchronized = [this, player_count, &expected_tick]() -> bool
  {
    for (int i = 0; i != player_count; ++i)
      if (m_clients[i].m_contest_runner->confirmed_tick() != expected_tick[i])
        return false;

    return true;
  };

  for (std::size_t t = 0; t != tick_count; ++t)
    {
      for (int i = 0; i != player_count; ++i)
        m_clients[i].tick(bim::game::contest::tick_interval);

      std::this_thread::sleep_for(std::chrono::seconds(0));
      m_scheduler.tick(std::chrono::milliseconds(20));
    }

  for (int i = 0; i != 100; ++i)
    {
      if (all_synchronized())
        return;

      std::this_thread::sleep_for(std::chrono::seconds(0));
      m_scheduler.tick(std::chrono::milliseconds(20));

      // Force a potential update from the server.
      for (int i = 0; i != player_count; ++i)
        m_clients[i].tick({});
    }
}

void new_game_after_game_over_test::tick()
{
  tick(bim::game::contest::tick_interval);
}

void new_game_after_game_over_test::wait(const std::function<bool()>& ready)
{
  for (int i = 0; i != 500; ++i)
    {
      std::this_thread::sleep_for(std::chrono::seconds(0));
      m_scheduler.tick(std::chrono::milliseconds(20));

      if (ready())
        break;
    }
}

/**
 * The players join a game, then when the game is over they all ask for another
 * game.
 */
TEST_P(new_game_after_game_over_test, game_over_then_new_random_game)
{
  authenticate();
  join_game();

  const int player_count = GetParam();
  const int last_player = player_count - 1;
  ASSERT_LE(0, last_player);

  for (int i = 0; i != player_count; ++i)
    ASSERT_NE(nullptr, m_clients[i].m_action) << "i=" << i;

  // Everyone except the last player drop a bomb.
  for (int i = 0; i != last_player; ++i)
    *m_clients[i].m_action =
        bim::game::player_action{ .movement = bim::game::player_movement::idle,
                                  .drop_bomb = true };

  *m_clients[last_player].m_action =
      bim::game::player_action{ .movement = bim::game::player_movement::idle,
                                .drop_bomb = false };

  tick();

  // Then wait for the bombs to to be applied, i.e. until they get out of the
  // queue.
  for (std::size_t tick_index = 0;
       tick_index != bim::game::player_action_queue::queue_size; ++tick_index)
    {
      for (int i = 0; i != player_count; ++i)
        *m_clients[i].m_action = {};

      tick();
    }

  // Wait three game-seconds for the bomb to explode.
  tick(std::chrono::seconds(3));
  // Keep running a bit to synchronize with the server.
  tick(std::chrono::seconds(1));

  // At this point all players except the last one are dead. The game should be
  // over.
  for (int i = 0; i < player_count; ++i)
    {
      EXPECT_FALSE(m_clients[i].m_result.still_running()) << "i=" << i;
      ASSERT_TRUE(m_clients[i].m_result.has_a_winner()) << "i=" << i;
      EXPECT_EQ(m_clients[last_player].m_player_index,
                m_clients[i].m_result.winning_player())
          << "i=" << i;
    }

  // The game is over, it's time for a new game.
  join_game();

  for (int i = 0; i != player_count; ++i)
    {
      ASSERT_TRUE(!!m_clients[i].m_started) << "i=" << i;
      EXPECT_TRUE(*m_clients[i].m_started) << "i=" << i;
    }
}

INSTANTIATE_TEST_SUITE_P(new_game_after_game_over_test_instance,
                         new_game_after_game_over_test,
                         testing::Values(2, 3, 4));
