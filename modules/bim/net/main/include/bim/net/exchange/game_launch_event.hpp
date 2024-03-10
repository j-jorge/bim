#pragma once

#include <iscool/net/message/channel_id.hpp>

namespace bim::net
{
  struct game_launch_event
  {
    iscool::net::channel_id channel;
    unsigned player_count;
    unsigned player_index;
  };
}
