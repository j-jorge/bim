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
#include <iscool/net/message_deserializer.h>
#include <iscool/net/socket_stream.h>

namespace bm::message
{
  class authentication;
}

namespace bm::server
{
  class server
  {
  public:
    explicit server(unsigned short port);

  private:
    void check_authentication(const iscool::net::endpoint& endpoint,
                              const bm::message::authentication& message);

  private:
    iscool::net::socket_stream m_socket;
    iscool::net::message_channel m_message_channel;
    iscool::net::message_deserializer m_message_deserializer;
  };
}
