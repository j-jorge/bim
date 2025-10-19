// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/feature_flags.hpp>

#include <bim/bit_map.impl.hpp>

#include <string_view>

namespace bim::app
{
  constexpr bim::bit_map<bim::game::feature_flags, std::string_view>
      g_feature_flag_name = []()
  {
    bim::bit_map<bim::game::feature_flags, std::string_view> result;

    result[bim::game::feature_flags::falling_blocks] = "falling_blocks";
    result[bim::game::feature_flags::fog_of_war] = "fog_of_war";
    result[bim::game::feature_flags::invisibility] = "invisibility";
    result[bim::game::feature_flags::shield] = "shield";

    return result;
  }();
}
