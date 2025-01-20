// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/feature_flags_fwd.hpp>

#include <iscool/net/message/channel_id.hpp>

#include <cstdint>

namespace bim::net
{
  struct game_launch_event
  {
    std::uint64_t seed;
    iscool::net::channel_id channel;
    unsigned player_count;
    unsigned player_index;
    bim::game::feature_flags features;
    std::uint8_t brick_wall_probability;
    std::uint8_t arena_width;
    std::uint8_t arena_height;
  };
}
