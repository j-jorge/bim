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

#include <iscool/net/message/channel_id.hpp>
#include <iscool/net/message/session_id.hpp>

#include <array>
#include <cstdint>

namespace bim::server
{
  struct game_info
  {
    std::uint64_t seed;
    iscool::net::channel_id channel;
    std::uint8_t player_count;
    std::array<iscool::net::session_id, 4> sessions;

    std::size_t session_index(iscool::net::session_id session) const;
  };
}
