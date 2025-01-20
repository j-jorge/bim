// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/feature_flags_fwd.hpp>

#include <bim/to_underlying.hpp>

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
