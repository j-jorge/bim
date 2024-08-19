// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/net/message/player_action_serialization.hpp>

#include <bim/game/component/player_action.hpp>
#include <bim/game/component/player_movement.hpp>

#include <iscool/net/byte_array_bit_inserter.hpp>
#include <iscool/net/byte_array_bit_reader.hpp>

#include <cassert>
#include <stdexcept>

static constexpr int g_bits_in_movement = 3;

void bim::net::read(bim::game::player_action& action,
                    iscool::net::byte_array_bit_reader& bits)
{
  const std::uint8_t movement = bits.get(g_bits_in_movement);

  if (movement > (int)bim::game::g_max_player_movement)
    throw std::runtime_error("");

  action.movement = (bim::game::player_movement)movement;
  action.drop_bomb = bits.get(1);
}

void bim::net::write(iscool::net::byte_array_bit_inserter& bits,
                     const bim::game::player_action& action)
{
  assert((int)action.movement <= ((1 << g_bits_in_movement) - 1));
  bits.append(action.movement, 3);
  bits.append(action.drop_bomb, 1);
}
