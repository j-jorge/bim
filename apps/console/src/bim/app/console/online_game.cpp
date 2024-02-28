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
#include <bim/net/exchange/game_update_exchange.hpp>
#include <bim/net/exchange/new_game_exchange.hpp>

#include <bim/game/contest.hpp>

#include <iscool/schedule/delayed_call.hpp>

#include <iostream>

bim::app::console::online_game::online_game(application& application,
                                            const std::string& host,
                                            const bim::net::game_name& name)
  : m_application(application)
  , m_socket_stream(host, iscool::net::socket_mode::client{})
  , m_message_stream(m_socket_stream)
  , m_authentication(m_message_stream)
{
  m_authentication.connect_to_authenticated(
      [this, name](iscool::net::session_id session) -> void
      {
        request_new_game(session, name);
      });
  m_authentication.connect_to_error(
      [&](bim::net::authentication_error_code error_code) -> void
      {
        std::cerr << "Authentication failed: " << (int)error_code << '\n';
        application.quit();
      });
  m_authentication.start();
}

bim::app::console::online_game::~online_game() = default;

void bim::app::console::online_game::request_new_game(
    iscool::net::session_id session, const bim::net::game_name& name)
{
  m_new_game.reset(new bim::net::new_game_exchange(m_message_stream, session));

  m_new_game->connect_to_game_proposal(
      [this](unsigned player_count) -> void
      {
        m_new_game->accept();
      });

  m_new_game->connect_to_launch_game(
      [this, session](iscool::net::channel_id channel, unsigned player_count,
                      unsigned player_index) -> void
      {
        launch_game(session, channel, player_count, player_index);
      });

  m_new_game->start(name);
}

void bim::app::console::online_game::launch_game(
    iscool::net::session_id session, iscool::net::channel_id channel,
    unsigned player_count, unsigned player_index)
{
  m_contest.reset(new bim::game::contest(1234, 80, player_count, 13, 11));
  m_game_channel.reset(
      new iscool::net::message_channel(m_message_stream, session, channel));
  m_update_exchange.reset(
      new bim::net::game_update_exchange(*m_game_channel, player_count));
  m_contest_runner.reset(new bim::net::contest_runner(
      *m_contest, *m_update_exchange, player_index, player_count));

  m_player_index = player_index;

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
