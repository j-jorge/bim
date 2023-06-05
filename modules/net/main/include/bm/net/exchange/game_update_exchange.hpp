/*
  Copyright (C) 2023 Julien Jorge

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <bm/net/exchange/server_update.hpp>

#include <bm/net/message/client_token.hpp>
#include <bm/net/message/game_update_from_client.hpp>

#include <bm/game/component/player_action.hpp>

#include <iscool/monitoring/declare_state_monitor.h>
#include <iscool/net/message/message.h>
#include <iscool/signals/declare_signal.h>
#include <iscool/signals/scoped_connection.h>

#include <array>

namespace iscool::net
{
  class message_channel;
}

namespace bm::net
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

    void push(const bm::game::player_action& action);

  private:
    void deserialize(const iscool::net::message& message);
    void dispatch_start();

    bool append_to_current_update(const bm::game::player_action& action);
    void send();

    void confirm_game_tick(const game_update_from_server& message);

    std::uint32_t
    validate_message(const game_update_from_server& message) const;

    void store_server_frames(const game_update_from_server& message,
                             std::uint32_t tick_count);

    void remove_server_confirmed_actions(std::uint32_t last_confirmed_tick);

  private:
    iscool::net::message_channel& m_message_channel;
    iscool::signals::scoped_connection m_channel_signal_connection;
    iscool::signals::scoped_connection m_send_connection;

    std::vector<bm::game::player_action> m_action_queue;
    game_update_from_client m_current_update;

    iscool::net::message m_client_out_message;

    server_update m_server_update;
    std::uint8_t m_player_count;

    ic_declare_state_monitor(m_monitor);
  };
}
