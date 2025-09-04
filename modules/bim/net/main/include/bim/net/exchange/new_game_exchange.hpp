// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/client_token.hpp>
#include <bim/net/message/encounter_id.hpp>
#include <bim/net/message/game_name.hpp>

#include <bim/game/feature_flags_fwd.hpp>

#include <iscool/monitoring/declare_state_monitor.hpp>
#include <iscool/net/message/message.hpp>
#include <iscool/net/message_channel.hpp>
#include <iscool/schedule/scoped_connection.hpp>
#include <iscool/signals/scoped_connection.hpp>

#include <optional>

namespace bim::net
{
  struct game_launch_event;

  class new_game_exchange
  {
    DECLARE_SIGNAL(void(unsigned), game_proposal, m_game_proposal)
    DECLARE_SIGNAL(void(const game_launch_event&), launch_game, m_launch_game)

  public:
    explicit new_game_exchange(const iscool::net::message_stream& stream);
    ~new_game_exchange();

    void start(iscool::net::session_id session, const game_name& name);
    void start(iscool::net::session_id session);
    void accept(bim::game::feature_flags features);
    void stop();

  private:
    void internal_start(iscool::net::session_id session);

    void tick();

    void interpret_received_message(const iscool::net::message& message);

    void check_on_hold(const iscool::net::message& m);
    void check_launch_game(const iscool::net::message& m);

  private:
    iscool::net::message_channel m_message_channel;

    iscool::signals::scoped_connection m_channel_signal_connection;
    iscool::schedule::scoped_connection m_update_connection;

    client_token m_token;
    iscool::net::message m_client_message;
    std::optional<encounter_id> m_encounter_id;

    ic_declare_state_monitor(m_monitor);
  };
}
