// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/test_client.hpp>

#include <bim/server/tests/fake_scheduler.hpp>

#include <bim/net/contest_runner.hpp>
#include <bim/net/exchange/game_launch_event.hpp>
#include <bim/net/exchange/game_update_exchange.hpp>

#include <bim/game/contest.hpp>
#include <bim/game/player_action.hpp>

#include <gtest/gtest.h>

bim::server::tests::test_client::test_client(
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

  m_new_game.connect_to_game_proposal(
      [this](unsigned) -> void
      {
        constexpr bim::game::feature_flags features{};
        m_new_game.accept(features);
      });

  m_new_game.connect_to_launch_game(
      [this, &message_stream](const bim::net::game_launch_event& event) -> void
      {
        launch_game(message_stream, event);
      });
}

bim::server::tests::test_client::~test_client() = default;

void bim::server::tests::test_client::authenticate()
{
  EXPECT_FALSE(!!m_session);

  m_authentication.start();

  for (int i = 0; (i != 10) && !m_session; ++i)
    m_scheduler.tick(std::chrono::milliseconds(20));
}

void bim::server::tests::test_client::new_game()
{
  ASSERT_TRUE(!!m_session);

  m_message_channel.reset();
  m_game_update.reset();
  contest.reset();
  started = std::nullopt;

  m_new_game.start(*m_session);
}

void bim::server::tests::test_client::launch_game(
    iscool::net::message_stream& stream,
    const bim::net::game_launch_event& event)
{
  EXPECT_TRUE(!!m_session);

  player_index = event.player_index;

  m_message_channel.reset(
      new iscool::net::message_channel(stream, *m_session, event.channel));
  m_game_update.reset(new bim::net::game_update_exchange(*m_message_channel,
                                                         event.player_count));
  contest.reset(new bim::game::contest(
      event.seed, event.brick_wall_probability, event.player_count,
      event.arena_width, event.arena_height, event.features));
  contest_runner.reset(new bim::net::contest_runner(
      *contest, *m_game_update, event.player_index, event.player_count));

  m_game_update->connect_to_started(
      [this]() -> void
      {
        started.emplace(true);
      });

  m_game_update->start();
}

void bim::server::tests::test_client::set_action(
    const bim::game::player_action& action)
{
  bim::game::player_action* const p = bim::game::find_player_action_by_index(
      contest->registry(), player_index);

  ASSERT_NE(nullptr, p) << "player_index=" << player_index;

  *p = action;
}

void bim::server::tests::test_client::tick(std::chrono::nanoseconds d)
{
  ASSERT_NE(nullptr, contest_runner);
  result = contest_runner->run(d);
}

bool bim::server::tests::test_client::is_in_game() const
{
  return !!started && *started;
}

void bim::server::tests::test_client::leave_game()
{
  started = std::nullopt;
}
