// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>
#include <string_view>

namespace bim::game
{
  enum class flame_direction : std::uint8_t;
  constexpr int flame_direction_count = 4;

  enum class flame_segment : std::uint8_t;
  constexpr int flame_segment_count = 3;

  bool is_horizontal(flame_direction d);
  bool is_vertical(flame_direction d);

  std::string_view to_string(flame_direction d);
  std::string_view to_string(flame_segment s);
}
