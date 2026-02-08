// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/client_server_simulator.hpp>

#include <bim/server/config.hpp>

#include <bim/net/contest_runner.hpp>

#include <bim/game/contest.hpp>

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

void bim::server::tests::client_server_simulator::join_game()
{
  for (int i = 0; i != m_player_count; ++i)
    clients[i].new_game();

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
          && (clients[i].contest_runner->confirmed_tick() != expected_tick[i]))
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
