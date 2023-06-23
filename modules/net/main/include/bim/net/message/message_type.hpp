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

#include <iscool/net/message/message_type.hpp>

namespace bim::net::message_type
{
  constexpr iscool::net::message_type authentication = 1;
  constexpr iscool::net::message_type authentication_ok = 2;
  constexpr iscool::net::message_type authentication_ko = 3;

  constexpr iscool::net::message_type new_game_request = 4;
  constexpr iscool::net::message_type game_on_hold = 5;
  constexpr iscool::net::message_type accept_game = 6;
  constexpr iscool::net::message_type launch_game = 7;

  constexpr iscool::net::message_type ready = 8;
  constexpr iscool::net::message_type start = 9;

  constexpr iscool::net::message_type game_update_from_client = 10;
  constexpr iscool::net::message_type game_update_from_server = 11;
}
