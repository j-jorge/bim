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

#include <bm/net/message/client_token.hpp>

#include <iscool/net/message_stream.h>

#include <unordered_map>

namespace bm::net
{
  class authentication;
}

namespace bm::server
{
  class authentication_service
  {
    DECLARE_SIGNAL(void(const iscool::net::endpoint&,
                        const iscool::net::message& message),
                   message, _message)

  public:
    explicit authentication_service(iscool::net::socket_stream& socket);
    ~authentication_service();

  private:
    using session_map
        = std::unordered_map<bm::net::client_token, iscool::net::session_id>;

    struct client_info;

    using client_map
        = std::unordered_map<iscool::net::session_id, client_info>;

  private:
    void check_session(const iscool::net::endpoint& endpoint,
                       const iscool::net::message& message);

    void check_authentication(const iscool::net::endpoint& endpoint,
                              const bm::net::authentication& message);

    iscool::signals::connection
    schedule_disconnection(iscool::net::session_id session);

  private:
    iscool::net::message_stream m_message_stream;
    iscool::net::session_id m_next_session_id;

    session_map m_sessions;
    client_map m_clients;
  };
}
