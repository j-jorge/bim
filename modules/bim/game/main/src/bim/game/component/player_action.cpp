#include <bim/game/component/player_action.hpp>

#include <cassert>

void bim::game::player_action::push(player_action_kind a)
{
  assert(queue_size < queue_capacity);

  queue[queue_size] = a;
  ++queue_size;
}

bool bim::game::player_action::full() const
{
  return queue_size == queue_capacity;
}
