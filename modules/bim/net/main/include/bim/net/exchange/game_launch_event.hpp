// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/game_id.hpp>

#include <bim/game/contest_fingerprint.hpp>

#include <iscool/net/message/channel_id.hpp>

#include <cstdint>

namespace bim::net
{
  struct game_launch_event
  {
    iscool::net::channel_id channel;
    bim::net::game_id game_id;
    bim::game::contest_fingerprint fingerprint;
    std::uint8_t player_index;
  };
}
