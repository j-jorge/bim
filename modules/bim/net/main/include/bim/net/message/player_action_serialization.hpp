// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

namespace iscool::net
{
  class byte_array_bit_reader;
  class byte_array_bit_inserter;
}

namespace bim::game
{
  struct player_action;
}

namespace bim::net
{
  void read(bim::game::player_action& action,
            iscool::net::byte_array_bit_reader& bits);
  void write(iscool::net::byte_array_bit_inserter& bits,
             const bim::game::player_action& action);
}
