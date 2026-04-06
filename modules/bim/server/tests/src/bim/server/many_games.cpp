// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/test_client.hpp>

#include <bim/server/tests/fake_scheduler.hpp>
#include <bim/server/tests/new_test_config.hpp>

#include <bim/server/server.hpp>

#include <bim/net/contest_runner.hpp>

#include <bim/game/bot.hpp>
#include <bim/game/component/player_action.hpp>
#include <bim/game/contest.hpp>
#include <bim/game/feature_flags.hpp>
#include <bim/game/player_action.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>

#include <format>
#include <iostream>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

class many_games_test : public testing::Test
{
public:
  many_games_test();

protected:
  bim::server::tests::fake_scheduler m_scheduler;

  const bim::server::config m_config;
  bim::server::server m_server;
  iscool::net::socket_stream m_socket_stream;
  iscool::net::message_stream m_message_stream;

  std::vector<std::unique_ptr<bim::server::tests::test_client>> m_clients;
  std::vector<bim::game::bot> m_bots;
};

many_games_test::many_games_test()
  : m_config(
        []()
          {
            bim::server::config config = bim::server::tests::new_test_config();

            config.enable_bots = true;
            config.matchmaking_delay_for_bot = std::chrono::seconds(1);
            config.matchmaking_delay_for_release = std::chrono::years(1);
            config.matchmaking_clean_up_interval = std::chrono::years(1);
            config.game_service_disconnection_inactivity_delay =
                std::chrono::years(1);
            config.game_service_clean_up_interval = std::chrono::years(1);
            config.session_clean_up_interval = std::chrono::years(1);

            return config;
          }())
  , m_server(m_config)
  , m_socket_stream("localhost:" + std::to_string(m_config.port),
                    iscool::net::socket_mode::client{})
  , m_message_stream(m_socket_stream)
{}

TEST_F(many_games_test, run)
{
  std::vector<bim::game::feature_flags> all_feature_flags_combined;
  all_feature_flags_combined.push_back({});

  for (bim::game::feature_flags f : bim::game::g_all_game_feature_flags)
    for (std::size_t i = 0, n = all_feature_flags_combined.size(); i != n; ++i)
      all_feature_flags_combined.push_back(all_feature_flags_combined[i] | f);

  std::size_t batch_size = 1;

  ic_log(iscool::log::nature::info(), "many_games_test",
         "Instantiating clients for {} games.",
         all_feature_flags_combined.size());

  // Create the clients and match them in a game for all combinations of the
  // game feature flags.
  for (const bim::game::feature_flags f : all_feature_flags_combined)
    {
      const std::size_t first_client = m_clients.size();

      for (std::size_t i = 0; i != batch_size; ++i)
        {
          m_clients.emplace_back(new bim::server::tests::test_client(
              m_scheduler, m_message_stream));

          m_clients.back()->authenticate();
        }

      for (std::size_t i = 0; i != batch_size; ++i)
        m_clients[first_client + i]->new_game_auto_accept(f);

      for (int i = 0; i != 10; ++i)
        {
          m_scheduler.tick(std::chrono::seconds(1));

          bool all_ready = true;

          for (std::size_t j = 0; j != batch_size; ++j)
            all_ready &= !!m_clients[first_client + j]->started
                         && *m_clients[first_client + j]->started;

          if (all_ready)
            break;
        }

      ASSERT_TRUE(!!m_clients[first_client]->game_launch_event)
          << "first_client=" << first_client;
      ASSERT_EQ(
          std::max<std::size_t>(2, batch_size),
          m_clients[first_client]->game_launch_event->fingerprint.player_count)
          << "first_client=" << first_client;

      for (std::size_t i = 0; i != batch_size; ++i)
        ASSERT_EQ(m_clients[first_client]->game_launch_event->channel,
                  m_clients[first_client + i]->game_launch_event->channel)
            << "first_client=" << first_client << ", i=" << i;

      ++batch_size;
      if (batch_size == 5)
        batch_size = 1;
    }

  const std::size_t client_count = m_clients.size();

  ic_log(iscool::log::nature::info(), "many_games_test",
         "Instantiating {} bots.", client_count);

  m_bots.reserve(client_count);

  for (std::size_t i = 0; i != client_count; ++i)
    m_bots.emplace_back(
        m_clients[i]->game_launch_event->player_index,
        m_clients[i]->game_launch_event->fingerprint.arena_width,
        m_clients[i]->game_launch_event->fingerprint.arena_height,
        m_clients[i]->game_launch_event->fingerprint.seed);

  bool all_done;
  std::size_t last_client_count = client_count;

  ic_log(iscool::log::nature::info(), "many_games_test", "Playing games.");

  do
    {
      bool updated = false;
      all_done = true;

      for (std::size_t i = 0; i != m_clients.size();)
        {
          const bool still_running = m_clients[i]->result.still_running();

          all_done &= !still_running;

          if (!still_running)
            {
              std::swap(m_clients[i], m_clients.back());
              m_clients.pop_back();
              continue;
            }

          updated = true;

          bim::game::player_action* const a =
              bim::game::find_player_action_by_index(
                  m_clients[i]->contest->registry(),
                  m_clients[i]->player_index);

          if (a)
            *a = m_bots[i].think(*m_clients[i]->contest);

          m_clients[i]->tick(bim::game::contest::tick_interval);

          ++i;
        }

      if (m_clients.size() != last_client_count)
        {
          ic_log(iscool::log::nature::info(), "many_games_test",
                 "{}/{} remaining clients.", m_clients.size(), client_count);
          last_client_count = m_clients.size();
        }

      if (!all_done)
        {
          if (!updated)
            for (std::size_t i = 0; i != m_clients.size(); ++i)
              {
                std::cout << std::format(
                    "Client[{}] still_running={}, tick={}/{}, session={}, "
                    "channel={}.\n",
                    i, m_clients[i]->result.still_running(),
                    m_clients[i]->contest_runner->local_tick(),
                    m_clients[i]->contest_runner->confirmed_tick(),
                    *m_clients[i]->session,
                    m_clients[i]->game_launch_event->channel);
              }

          ASSERT_TRUE(updated);

          m_scheduler.tick(std::chrono::milliseconds(50));
        }
    }
  while (!all_done);
}
