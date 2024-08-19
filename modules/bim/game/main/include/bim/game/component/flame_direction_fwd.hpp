// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>

namespace bim::game
{
  enum class flame_direction : std::uint8_t;
  enum class flame_segment : std::uint8_t;

  bool is_horizontal(flame_direction d);
  bool is_vertical(flame_direction d);
}
