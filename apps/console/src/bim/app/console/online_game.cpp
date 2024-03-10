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
#include <bim/app/console/online_game.hpp>

#include <bim/app/console/application.hpp>
#include <bim/app/console/display.hpp>
#include <bim/app/console/inputs.hpp>

#include <bim/net/contest_runner.hpp>
#include <bim/net/exchange/game_launch_event.hpp>
#include <bim/net/exchange/game_update_exchange.hpp>

#include <bim/game/contest.hpp>

#include <iscool/schedule/delayed_call.hpp>

#include <iostream>

bim::app::console::online_game::online_game(application& application,
                                            const std::string& host,
                                            const bim::net::game_name& name)
  : m_application(application)
  , m_new_game(m_session.message_stream())
{
  m_session.connect_to_connected(
      [this, name]() -> void
      {
        m_new_game.start(m_session.session_id(), name);
      });
  m_session.connect_to_authentication_error(
      [&](bim::net::authentication_error_code error_code) -> void
      {
        std::cerr << "Authentication failed: " << (int)error_code << '\n';
        application.quit();
      });
  m_session.connect(host);

  m_new_game.connect_to_game_proposal(
      [this](unsigned player_count) -> void
      {
        m_new_game.accept();
      });

  m_new_game.connect_to_launch_game(
      [this](const bim::net::game_launch_event& event) -> void
      {
        launch_game(event);
      });
}

bim::app::console::online_game::~online_game() = default;

void bim::app::console::online_game::launch_game(
    const bim::net::game_launch_event& event)
{
  m_contest.reset(
      new bim::game::contest(1234, 80, event.player_count, 13, 15));
  m_game_channel.reset(new iscool::net::message_channel(
      m_session.message_stream(), m_session.session_id(), event.channel));
  m_update_exchange.reset(
      new bim::net::game_update_exchange(*m_game_channel, event.player_count));
  m_contest_runner.reset(new bim::net::contest_runner(
      *m_contest, *m_update_exchange, event.player_index, event.player_count));

  m_player_index = event.player_index;

  m_update_exchange->connect_to_started(
      [this]() -> void
      {
        m_input_thread = launch_input_thread(m_input);
        schedule_tick();
      });
  m_update_exchange->start();
}

void bim::app::console::online_game::schedule_tick()
{
  m_tick_connection = iscool::schedule::delayed_call(
      [this]() -> void
      {
        tick();
      },
      m_application.update_interval());
}

void bim::app::console::online_game::tick()
{
  if (!apply_inputs(m_contest->registry(), m_player_index,
                    m_input.exchange(0)))
    m_application.quit();
  else
    {
      m_contest_runner->run(m_application.update_interval());
      display(*m_contest);
      schedule_tick();
    }
}
