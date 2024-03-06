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

#include <bim/net/contest_runner.hpp>
#include <bim/net/exchange/new_game_exchange.hpp>
#include <bim/net/message/game_name.hpp>
#include <bim/net/session_handler.hpp>

#include <iscool/signals/scoped_connection.hpp>

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>

namespace iscool::net
{
  class message_channel;
}

namespace bim::net
{
  class contest_runner;
  class game_update_exchange;
}

namespace bim::app::console
{
  class application;

  class online_game
  {
  public:
    online_game(application& application, const std::string& host,
                const bim::net::game_name& name);
    ~online_game();

  private:
    void launch_game(iscool::net::channel_id channel, unsigned player_count,
                     unsigned player_index);

    void schedule_tick();
    void tick();

  private:
    application& m_application;

    bim::net::session_handler m_session;
    bim::net::new_game_exchange m_new_game;

    std::unique_ptr<bim::game::contest> m_contest;
    std::unique_ptr<iscool::net::message_channel> m_game_channel;
    std::unique_ptr<bim::net::game_update_exchange> m_update_exchange;
    std::unique_ptr<bim::net::contest_runner> m_contest_runner;
    std::uint8_t m_player_index;

    std::atomic<int> m_input;
    std::jthread m_input_thread;

    iscool::signals::scoped_connection m_tick_connection;
  };
}
