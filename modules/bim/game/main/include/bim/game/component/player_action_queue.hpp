#pragma once

#include <bim/game/component/player_action.hpp>

namespace bim::game
{
  struct queued_action
  {
    player_action action;

    /// Where the player was when the action was scheduled.
    uint8_t arena_x;
    uint8_t arena_y;
  };

  struct player_action_queue
  {
    queued_action enqueue(player_action a, uint8_t arena_x, uint8_t arena_y);

    static constexpr std::size_t queue_size = 8;
    queued_action m_queue[queue_size];
  };
}
