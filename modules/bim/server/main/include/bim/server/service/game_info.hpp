// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/constant/max_player_count.hpp>

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
    std::array<iscool::net::session_id, bim::game::g_max_player_count>
        sessions;

    std::size_t session_index(iscool::net::session_id session) const;
  };
}
