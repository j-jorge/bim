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

#include <iscool/net/message/message.h>
#include <iscool/net/message_channel.h>
#include <iscool/net/message_deserializer.h>
#include <iscool/signals/scoped_connection.h>

namespace iscool::net
{
  class message_stream;
}

namespace bm::net
{
  template <typename ClientMessage, typename AnswerOK, typename AnswerKO>
  class exchange
  {
    DECLARE_SIGNAL(void(const iscool::net::endpoint&, const AnswerOK), ok, _ok)
    DECLARE_SIGNAL(void(const iscool::net::endpoint&, const AnswerKO), ko, _ko)

  public:
    exchange(iscool::net::message_stream& stream,
             iscool::net::session_id session, iscool::net::channel_id channel);

    void start(const ClientMessage& message);
    void stop();

  private:
    void tick();

  private:
    iscool::net::message_channel m_message_channel;
    iscool::net::message_deserializer m_deserializer;
    iscool::signals::scoped_connection m_channel_signal_connection;
    iscool::signals::scoped_connection m_update_connection;

    iscool::net::message m_client_message;
  };
}
