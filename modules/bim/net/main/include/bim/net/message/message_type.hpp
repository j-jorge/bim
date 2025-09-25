// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/net/message/message_type.hpp>

namespace bim::net::message_type
{
  constexpr iscool::net::message_type authentication = 1;
  constexpr iscool::net::message_type authentication_ok = 2;
  constexpr iscool::net::message_type authentication_ko = 3;
  constexpr iscool::net::message_type keep_alive = 4;
  constexpr iscool::net::message_type acknowledge_keep_alive = 5;
  constexpr iscool::net::message_type hello = 6;
  constexpr iscool::net::message_type hello_ok = 7;

  constexpr iscool::net::message_type new_named_game_request = 10;
  constexpr iscool::net::message_type new_random_game_request = 11;

  constexpr iscool::net::message_type game_on_hold = 20;
  constexpr iscool::net::message_type accept_named_game = 21;
  constexpr iscool::net::message_type accept_random_game = 22;
  constexpr iscool::net::message_type launch_game = 23;

  constexpr iscool::net::message_type ready = 24;
  constexpr iscool::net::message_type start = 25;

  constexpr iscool::net::message_type game_update_from_client = 26;
  constexpr iscool::net::message_type game_update_from_server = 27;

  constexpr iscool::net::message_type game_over = 28;
}
