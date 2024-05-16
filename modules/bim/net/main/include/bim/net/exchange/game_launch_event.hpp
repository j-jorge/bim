#pragma once

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
  };
}
