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

#include <bim/net/message/client_token.hpp>
#include <bim/net/message/encounter_id.hpp>
#include <bim/net/message/game_name.hpp>

#include <iscool/monitoring/declare_state_monitor.hpp>
#include <iscool/net/message/message.hpp>
#include <iscool/net/message_channel.hpp>
#include <iscool/net/message_deserializer.hpp>
#include <iscool/signals/scoped_connection.hpp>

namespace bim::net
{
  class game_on_hold;
  class launch_game;

  class new_game_exchange
  {
    DECLARE_SIGNAL(void(unsigned), game_proposal, m_game_proposal)
    DECLARE_SIGNAL(void(iscool::net::channel_id, unsigned, unsigned),
                   launch_game, m_launch_game)

  public:
    new_game_exchange(iscool::net::message_stream& stream,
                      iscool::net::session_id session);
    ~new_game_exchange();

    void start(const game_name& name);
    void accept();
    void stop();

  private:
    void tick();

    void check_on_hold(const game_on_hold& message);
    void check_launch_game(const launch_game& message);

  private:
    iscool::net::message_channel m_message_channel;
    iscool::net::message_deserializer m_deserializer;

    iscool::signals::scoped_connection m_channel_signal_connection;
    iscool::signals::scoped_connection m_update_connection;
    iscool::signals::scoped_connection m_deserializer_connection;

    client_token m_token;
    iscool::net::message m_client_message;
    std::optional<encounter_id> m_encounter_id;

    ic_declare_state_monitor(m_monitor);
  };
}
