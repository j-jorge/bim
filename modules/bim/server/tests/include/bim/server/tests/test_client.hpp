// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/exchange/authentication_exchange.hpp>
#include <bim/net/exchange/new_game_exchange.hpp>

#include <bim/game/contest_result.hpp>

#include <iscool/net/message/session_id.hpp>

#include <chrono>
#include <memory>
#include <optional>

namespace iscool::net
{
  class message_channel;
}

namespace bim
{
  namespace game
  {
    class contest;
    struct player_action;
  }

  namespace net
  {
    class contest_runner;
    class game_update_exchange;
  }
}

namespace bim::server::tests
{
  class fake_scheduler;

  class test_client
  {
  public:
    test_client(bim::server::tests::fake_scheduler& scheduler,
                iscool::net::message_stream& message_stream);
    ~test_client();

    void authenticate();
    void new_game();
    void tick(std::chrono::nanoseconds d);

    bool is_in_game() const;
    void leave_game();

    void set_action(const bim::game::player_action& action);

  public:
    std::optional<bool> started;
    std::unique_ptr<bim::net::contest_runner> contest_runner;
    std::unique_ptr<bim::game::contest> contest;
    bim::game::contest_result result;
    int player_index;

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
  };
}
