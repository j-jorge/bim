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

#include <bim/net/message/authentication_error_code.hpp>
#include <bim/net/message/client_token.hpp>

#include <iscool/net/message/message.h>
#include <iscool/net/message_channel.h>
#include <iscool/net/message_deserializer.h>
#include <iscool/signals/scoped_connection.h>

namespace bim::net
{
  class authentication_ok;
  class authentication_ko;

  class authentication_exchange
  {
    DECLARE_SIGNAL(void(iscool::net::session_id), authenticated,
                   m_authenticated)
    DECLARE_SIGNAL(void(authentication_error_code), error, m_error)

  public:
    explicit authentication_exchange(iscool::net::message_stream& stream);
    ~authentication_exchange();

    void start();
    void stop();

  private:
    void tick();

    void check_ok(const authentication_ok& message);
    void check_ko(const authentication_ko& message);

  private:
    iscool::net::message_channel m_message_channel;
    iscool::net::message_deserializer m_deserializer;
    iscool::signals::scoped_connection m_channel_signal_connection;
    iscool::signals::scoped_connection m_update_connection;

    client_token m_token;
    iscool::net::message m_client_message;
  };
}
