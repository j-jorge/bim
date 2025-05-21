// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/feature_flags.hpp>

#include <bim/to_underlying.hpp>

const std::array<bim::game::feature_flags, 3>
    bim::game::g_all_game_feature_flags = {
      bim::game::feature_flags::falling_blocks,
      bim::game::feature_flags::invisibility,
      bim::game::feature_flags::fog_of_war
    };

bim::game::feature_flags& bim::game::operator|=(feature_flags& lhs,
                                                feature_flags rhs)
{
  lhs = (feature_flags)(bim::to_underlying(lhs) | bim::to_underlying(rhs));
  return lhs;
}

bim::game::feature_flags bim::game::operator|(feature_flags lhs,
                                              feature_flags rhs)
{
  return lhs |= rhs;
}

bim::game::feature_flags& bim::game::operator&=(feature_flags& lhs,
                                                feature_flags rhs)
{
  lhs = (feature_flags)(bim::to_underlying(lhs) & bim::to_underlying(rhs));
  return lhs;
}

bim::game::feature_flags bim::game::operator&(feature_flags lhs,
                                              feature_flags rhs)
{
  return lhs &= rhs;
}

bim::game::feature_flags& bim::game::operator^=(feature_flags& lhs,
                                                feature_flags rhs)
{
  lhs = (feature_flags)(bim::to_underlying(lhs) ^ bim::to_underlying(rhs));
  return lhs;
}

bim::game::feature_flags bim::game::operator^(feature_flags lhs,
                                              feature_flags rhs)
{
  return lhs ^= rhs;
}

bim::game::feature_flags bim::game::operator~(feature_flags f)
{
  return (feature_flags)~bim::to_underlying(f);
}

bool bim::game::operator!(feature_flags f)
{
  return !bim::to_underlying(f);
}
