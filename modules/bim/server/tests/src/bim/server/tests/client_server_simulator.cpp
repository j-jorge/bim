// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/client_server_simulator.hpp>

#include <bim/server/config.hpp>

#include <bim/net/contest_runner.hpp>

#include <bim/game/component/player_action_queue.hpp>
#include <bim/game/component/player_movement.hpp>
#include <bim/game/contest.hpp>
#include <bim/game/player_action.hpp>

#include <iscool/log/setup.hpp>
#include <iscool/net/message_channel.hpp>

#include <gtest/gtest.h>

bim::server::tests::client_server_simulator::client_server_simulator(
    std::uint8_t player_count, const bim::server::config& config)
  : m_player_count(player_count)
  , m_server(config)
  , m_socket_stream("localhost:" + std::to_string(config.port),
                    iscool::net::socket_mode::client{})
  , m_message_stream(m_socket_stream)
  , config(config)
  , clients{ bim::server::tests::test_client(m_scheduler, m_message_stream),
             bim::server::tests::test_client(m_scheduler, m_message_stream),
             bim::server::tests::test_client(m_scheduler, m_message_stream),
             bim::server::tests::test_client(m_scheduler, m_message_stream) }
{}

bim::server::tests::client_server_simulator::~client_server_simulator() =
    default;

void bim::server::tests::client_server_simulator::authenticate()
{
  for (int i = 0; i != m_player_count; ++i)
    {
      clients[i].authenticate();
      EXPECT_TRUE(!!clients[i].session) << "i=" << i;
    }
}

void bim::server::tests::client_server_simulator::
    authenticate_with_business_token()
{
  for (int i = 0; i != m_player_count; ++i)
    {
      clients[i].user_id = i + 1;
      // The mockup for the business server will parse the session token to get
      // the user ID.
      const std::string user_id_str = std::to_string(clients[i].user_id);
      clients[i].authenticate(
          bim::net::session_token(user_id_str.begin(), user_id_str.end()));

      EXPECT_TRUE(!!clients[i].session) << "i=" << i;
    }
}

void bim::server::tests::client_server_simulator::join_game()
{
  for (int i = 0; i != m_player_count; ++i)
    clients[i].new_game_auto_accept();

  // Let the time pass such that the messages can move between the clients and
  // the server.
  wait(
      [this]() -> bool
        {
          for (int i = 0; i != m_player_count; ++i)
            if (!clients[i].started)
              return false;

          return true;
        });

  for (int i = 0; i != m_player_count; ++i)
    EXPECT_TRUE(clients[i].is_in_game()) << "i=" << i;
}

void bim::server::tests::client_server_simulator::tick(
    std::chrono::nanoseconds d)
{
  const std::size_t tick_count = d / bim::game::contest::tick_interval
                                 + (d % bim::game::contest::tick_interval
                                    != std::chrono::nanoseconds::zero());
  tick(tick_count);
}

void bim::server::tests::client_server_simulator::tick(std::size_t tick_count)
{
  std::array<std::size_t, 4> expected_tick;

  for (int i = 0; i != m_player_count; ++i)
    if (clients[i].is_in_game())
      expected_tick[i] =
          clients[i].contest_runner->confirmed_tick() + tick_count;

  const auto all_synchronized = [this, &expected_tick]() -> bool
    {
      for (int i = 0; i != m_player_count; ++i)
        if (clients[i].is_in_game()
            && (clients[i].contest_runner->confirmed_tick()
                != expected_tick[i]))
          return false;

      return true;
    };

  for (std::size_t t = 0; t != tick_count; ++t)
    {
      for (int i = 0; i != m_player_count; ++i)
        if (clients[i].is_in_game())
          clients[i].tick(bim::game::contest::tick_interval);

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
      for (int i = 0; i != m_player_count; ++i)
        if (clients[i].is_in_game())
          clients[i].tick({});
    }
}

void bim::server::tests::client_server_simulator::tick(int client_index,
                                                       std::size_t tick_count)
{
  const std::size_t expected_tick =
      clients[client_index].contest_runner->confirmed_tick() + tick_count;

  for (std::size_t t = 0; t != tick_count; ++t)
    {
      clients[client_index].tick(bim::game::contest::tick_interval);
      std::this_thread::sleep_for(std::chrono::seconds(0));
      m_scheduler.tick(std::chrono::milliseconds(20));
    }

  for (int i = 0; i != 100; ++i)
    {
      if (clients[client_index].contest_runner->confirmed_tick()
          == expected_tick)
        return;

      std::this_thread::sleep_for(std::chrono::seconds(0));
      m_scheduler.tick(std::chrono::milliseconds(20));

      // Force a potential update from the server.
      clients[client_index].tick({});
    }
}

void bim::server::tests::client_server_simulator::tick()
{
  tick(bim::game::contest::tick_interval);
}

void bim::server::tests::client_server_simulator::wait(
    std::chrono::milliseconds d)
{
  std::this_thread::sleep_for(std::chrono::seconds(0));
  m_scheduler.tick(d);
}

void bim::server::tests::client_server_simulator::wait(
    const std::function<bool()>& ready)
{
  for (int i = 0; i != 500; ++i)
    {
      std::this_thread::sleep_for(std::chrono::seconds(0));
      m_scheduler.tick(std::chrono::milliseconds(20));

      if (ready())
        break;
    }
}

void bim::server::tests::client_server_simulator::drop_bombs(
    std::uint8_t client_index_mask)
{
  // Everyone except the last player drop a bomb.
  for (int i = 0; i != m_player_count; ++i)
    if (client_index_mask & (1 << i))
      clients[i].set_action(bim::game::player_action{
          .movement = bim::game::player_movement::idle, .drop_bomb = true });
    else
      clients[i].set_action(bim::game::player_action{
          .movement = bim::game::player_movement::idle, .drop_bomb = false });

  tick();

  // Then wait for the bombs to to be applied, i.e. until they get out of the
  // queue.
  for (std::size_t tick_index = 0;
       tick_index != bim::game::player_action_queue::queue_size; ++tick_index)
    {
      for (int i = 0; i != m_player_count; ++i)
        clients[i].set_action({});

      tick();
    }
}

void bim::server::tests::client_server_simulator::wait_game_over()
{
  // Wait for the game to end.
  const auto still_running = [this]() -> bool
    {
      for (int i = 0; i != m_player_count; ++i)
        if (clients[i].started && clients[i].result.still_running())
          return true;

      return false;
    };

  int d = std::chrono::seconds(bim::game::contest::max_game_duration).count();

  // Try a quick loop for the cases where the game over is expected soon.
  for (int i = 0; (i != 10) && still_running(); ++i, --d)
    tick(std::chrono::seconds(1));

  // Then do larger iterations if the game is still not over.
  for (; (d > 60) && still_running(); d -= 60)
    tick(std::chrono::minutes(1));

  for (; (d > 10) && still_running(); d -= 10)
    tick(std::chrono::seconds(10));

  for (; (d > 0) && still_running(); --d)
    tick(std::chrono::seconds(1));
}
