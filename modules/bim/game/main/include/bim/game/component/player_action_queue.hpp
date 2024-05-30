#pragma once

#include <bim/game/component/player_action.hpp>

namespace bim::game
{
  struct player_action_queue
  {
    player_action enqueue(player_action a);

    static constexpr std::size_t queue_size = 8;
    player_action m_queue[queue_size];
  };
}
