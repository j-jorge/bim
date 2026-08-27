// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/business/player_profile.hpp>

bim::game::feature_flags
bim::app::player_profile::enabled_feature_flags() const
{
  bim::game::feature_flags flags{};

  for (const bim::game::feature_flags f : slot_feature)
    flags |= f;

  return flags;
}
