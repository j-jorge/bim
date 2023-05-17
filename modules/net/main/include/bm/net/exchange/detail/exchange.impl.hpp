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

#include <iscool/net/message_channel.h>
#include <iscool/net/message_deserializer.impl.tpp>
#include <iscool/schedule/delayed_call.h>
#include <iscool/signals/implement_signal.h>
#include <iscool/signals/relay.h>

template <typename ClientMessage, typename AnswerOK, typename AnswerKO>
IMPLEMENT_SIGNAL((bm::net::exchange<ClientMessage, AnswerOK, AnswerKO>), ok,
                 _ok);

template <typename ClientMessage, typename AnswerOK, typename AnswerKO>
IMPLEMENT_SIGNAL((bm::net::exchange<ClientMessage, AnswerOK, AnswerKO>), ko,
                 _ko);

template <typename ClientMessage, typename AnswerOK, typename AnswerKO>
bm::net::exchange<ClientMessage, AnswerOK, AnswerKO>::exchange(
    iscool::net::message_stream& stream, iscool::net::session_id session,
    iscool::net::channel_id channel)
  : m_message_channel(stream, session, channel)
{
  m_deserializer.connect_signal<AnswerOK>(iscool::signals::relay(_ok));
  m_deserializer.connect_signal<AnswerKO>(iscool::signals::relay(_ko));
}

template <typename ClientMessage, typename AnswerOK, typename AnswerKO>
void bm::net::exchange<ClientMessage, AnswerOK, AnswerKO>::start(
    const ClientMessage& message)
{
  m_client_message = message.build_message();

  m_channel_signal_connection = m_message_channel.connect_to_message(std::bind(
      &iscool::net::message_deserializer::interpret_received_message,
      &m_deserializer, std::placeholders::_1, std::placeholders::_2));

  tick();
}

template <typename ClientMessage, typename AnswerOK, typename AnswerKO>
void bm::net::exchange<ClientMessage, AnswerOK, AnswerKO>::stop()
{
  m_channel_signal_connection.disconnect();
  m_update_connection.disconnect();
}

template <typename ClientMessage, typename AnswerOK, typename AnswerKO>
void bm::net::exchange<ClientMessage, AnswerOK, AnswerKO>::tick()
{
  iscool::schedule::delayed_call(std::bind(&exchange::tick, this),
                                 std::chrono::seconds(1));
  m_message_channel.send(m_client_message);
}
