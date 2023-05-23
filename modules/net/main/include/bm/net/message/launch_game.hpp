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
#include <bm/net/message/game_id.hpp>
#include <bm/net/message/message_type.hpp>

#include <iscool/net/message/raw_message.h>

namespace bm::net
{
  DECLARE_RAW_MESSAGE(launch_game, message_type::launch_game,
                      ((client_token)(request_token)) //
                      ((std::uint8_t)(player_count))  //
                      ((iscool::net::channel_id)(game_channel)));
}
