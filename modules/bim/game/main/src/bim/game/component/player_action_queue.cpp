#include <bim/game/component/player_action_queue.hpp>

bim::game::queued_action
bim::game::player_action_queue::enqueue(player_action a, uint8_t arena_x,
                                        uint8_t arena_y)
{
  const queued_action result = m_queue[0];

  for (std::size_t i = 1; i < queue_size; ++i)
    m_queue[i - 1] = m_queue[i];

  m_queue[queue_size - 1] =
      queued_action{ .action = a, .arena_x = arena_x, .arena_y = arena_y };

  return result;
}
