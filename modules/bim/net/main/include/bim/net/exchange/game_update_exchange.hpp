// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/exchange/server_update.hpp>

#include <bim/net/message/client_token.hpp>
#include <bim/net/message/game_update_from_client.hpp>

#include <bim/game/component/player_action.hpp>

#include <iscool/monitoring/declare_state_monitor.hpp>
#include <iscool/net/message/message.hpp>
#include <iscool/signals/declare_signal.hpp>
#include <iscool/signals/scoped_connection.hpp>

#include <array>

namespace iscool::net
{
  class message_channel;
}

namespace bim::net
{
  class game_update_from_server;

  class game_update_exchange
  {
    DECLARE_VOID_SIGNAL(started, m_started)
    DECLARE_SIGNAL(void(const server_update&), updated, m_updated)

  public:
    game_update_exchange(iscool::net::message_channel& channel,
                         std::uint8_t player_count);
    ~game_update_exchange();

    void start();

    void push(const bim::game::player_action& action);

  private:
    void deserialize(const iscool::net::message& message);
    void dispatch_start();

    bool append_to_current_update(const bim::game::player_action& action);
    void send();

    void confirm_game_tick(const iscool::net::message& m);

    std::uint32_t
    validate_message(const game_update_from_server& message) const;

    void store_server_frames(const game_update_from_server& message,
                             std::uint32_t tick_count);

    void remove_server_confirmed_actions(std::uint32_t last_confirmed_tick);

  private:
    iscool::net::message_channel& m_message_channel;
    iscool::signals::scoped_connection m_channel_signal_connection;
    iscool::signals::scoped_connection m_send_connection;

    std::vector<bim::game::player_action> m_action_queue;
    game_update_from_client m_current_update;

    iscool::net::message m_client_out_message;

    server_update m_server_update;
    std::uint8_t m_player_count;

    ic_declare_state_monitor(m_monitor);
  };
}
