// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/feature_flags_string.hpp>

#include <bim/game/feature_flags.hpp>

#include <bim/assume.hpp>

std::string_view bim::game::to_simple_string(feature_flags f)
{
  switch (f)
    {
    case feature_flags::falling_blocks:
      return "falling_blocks";
    case feature_flags::fog_of_war:
      return "fog_of_war";
    case feature_flags::invisibility:
      return "invisibility";
    case feature_flags::shield:
      return "shield";
    case feature_flags::fences:
      return "fences";
    }

  bim_assume(false);

  return "?";
}

std::optional<bim::game::feature_flags>
bim::game::from_simple_string(std::string_view s)
{
  for (feature_flags f : g_all_game_feature_flags)
    if (s == to_simple_string(f))
      return f;

  return std::nullopt;
}
