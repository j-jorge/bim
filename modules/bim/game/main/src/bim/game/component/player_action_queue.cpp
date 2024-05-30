#include <bim/game/component/player_action_queue.hpp>

bim::game::player_action
bim::game::player_action_queue::enqueue(player_action a)
{
  const player_action result = m_queue[0];

  for (std::size_t i = 1; i < queue_size; ++i)
    m_queue[i - 1] = m_queue[i];

  m_queue[queue_size - 1] = a;

  return result;
}
