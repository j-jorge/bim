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

#include <bim/net/message/client_token.hpp>
#include <bim/net/message/game_name.hpp>
#include <bim/net/message/message_type.hpp>

#include <iscool/net/byte_array_serialization/byte_array_array_serialization.hpp>
#include <iscool/net/message/raw_message.hpp>

namespace bim::net
{
  DECLARE_RAW_MESSAGE(new_game_request, message_type::new_game_request,
                      ((client_token)(request_token)) //
                      ((game_name)(name)));
}
